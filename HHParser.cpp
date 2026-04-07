#include "HHParser.h"
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <regex>
#include <windows.h>
#include <winhttp.h>
#include <sstream>
#pragma comment(lib, "winhttp.lib")
using namespace std;

string urlEncode(const string& str) {
    string encoded;
    for (unsigned char c : str) {
        if (c >= 'a' && c <= 'z') encoded += c;
        else if (c >= 'A' && c <= 'Z') encoded += c;
        else if (c >= '0' && c <= '9') encoded += c;
        else if (c == ' ') encoded += '+';
        else if (c == '-' || c == '_' || c == '.' || c == '~') encoded += c;
        else {
            char buf[10];
            sprintf_s(buf, "%%%02X", c);
            encoded += buf;
        }
    }
    return encoded;
}

HHParser::HHParser(int pages) : pages(pages) {}

string HHParser::buildUrl(const string& keyword, int page) const {
    string encodedKeyword = urlEncode(keyword);
    return string(BASE_URL) + "?text=" + encodedKeyword +
        "&page=" + to_string(page) +
        "&per_page=" + to_string(VACANCIES_PER_PAGE);
}

string HHParser::sendHttpRequest(const string& url) const {
    string result;

    HINTERNET hSession = WinHttpOpen(L"HH Parser/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        NULL, NULL, 0);
    if (!hSession) return "";

    // ВАЖНО: подключаемся к api.hh.ru
    HINTERNET hConnect = WinHttpConnect(hSession, L"api.hh.ru", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Извлекаем путь из URL
    string path;
    size_t pos = url.find("/vacancies");
    if (pos != string::npos) {
        path = url.substr(pos);
    }
    else {
        path = "/vacancies";
    }

    wstring wpath(path.begin(), path.end());

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wpath.c_str(), NULL, NULL, NULL, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    DWORD timeout = 10000;
    WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

    LPCWSTR headers = L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\r\nAccept: application/json\r\n";

    BOOL bResults = WinHttpSendRequest(hRequest, headers, wcslen(headers), NULL, 0, 0, 0);
    if (bResults) bResults = WinHttpReceiveResponse(hRequest, NULL);

    if (bResults) {
        DWORD dwSize = 0;
        do {
            dwSize = 0;
            WinHttpQueryDataAvailable(hRequest, &dwSize);
            if (dwSize > 0) {
                char* buffer = new char[dwSize + 1];
                ZeroMemory(buffer, dwSize + 1);
                DWORD dwRead = 0;
                if (WinHttpReadData(hRequest, buffer, dwSize, &dwRead)) {
                    result.append(buffer, dwRead);
                }
                delete[] buffer;
            }
        } while (dwSize > 0);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return result;
}

void HHParser::respectRateLimit() const {
    static auto lastRequest = chrono::steady_clock::now();
    auto now = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastRequest);
    if (elapsed.count() < 1000) {
        this_thread::sleep_for(chrono::milliseconds(1000 - elapsed.count()));
    }
    lastRequest = chrono::steady_clock::now();
}

vector<Vacancy> HHParser::parsePage(const string& jsonData, int page) {
    vector<Vacancy> vacancies;

    if (jsonData.empty()) {
        return vacancies;
    }

    rapidjson::Document document;
    document.Parse(jsonData.c_str());

    if (document.HasParseError()) {
        return vacancies;
    }

    if (!document.HasMember("items") || !document["items"].IsArray()) {
        return vacancies;
    }

    const auto& items = document["items"];
    for (rapidjson::SizeType i = 0; i < items.Size(); ++i) {
        const auto& v = items[i];

        string id = "";
        if (v.HasMember("id") && v["id"].IsString()) {
            id = v["id"].GetString();
        }

        string title = "";
        if (v.HasMember("name") && v["name"].IsString()) {
            title = v["name"].GetString();
        }

        string company = "Не указана";
        if (v.HasMember("employer") && v["employer"].IsObject()) {
            const auto& employer = v["employer"];
            if (employer.HasMember("name") && employer["name"].IsString()) {
                company = employer["name"].GetString();
            }
        }

        int salary_from = 0, salary_to = 0;
        bool has_from = false, has_to = false;

        if (v.HasMember("salary") && v["salary"].IsObject() && !v["salary"].IsNull()) {
            const auto& salary = v["salary"];
            if (salary.HasMember("from") && salary["from"].IsInt()) {
                salary_from = salary["from"].GetInt();
                has_from = true;
            }
            if (salary.HasMember("to") && salary["to"].IsInt()) {
                salary_to = salary["to"].GetInt();
                has_to = true;
            }
        }

        string description = "";
        if (v.HasMember("snippet") && v["snippet"].IsObject()) {
            const auto& snippet = v["snippet"];
            if (snippet.HasMember("requirement") && snippet["requirement"].IsString()) {
                string temp = snippet["requirement"].GetString();
                regex tagRegex("<[^>]*>");
                description = regex_replace(temp, tagRegex, "");
                regex spaceRegex("\\s+");
                description = regex_replace(description, spaceRegex, " ");
                if (description.length() > 500) {
                    description = description.substr(0, 497) + "...";
                }
            }
        }

        string url = "";
        if (v.HasMember("alternate_url") && v["alternate_url"].IsString()) {
            url = v["alternate_url"].GetString();
        }

        string city = "Не указан";
        if (v.HasMember("area") && v["area"].IsObject()) {
            const auto& area = v["area"];
            if (area.HasMember("name") && area["name"].IsString()) {
                city = area["name"].GetString();
            }
        }

        Vacancy vac(id, title, company, salary_from, salary_to,
            has_from, has_to, description, url, city);
        vacancies.push_back(vac);
    }
    return vacancies;
}

vector<Vacancy> HHParser::parse(const string& keyword) {
    vector<Vacancy> allVacancies;

    cout << "Поиск по запросу: " << keyword << endl;
    cout << "Страниц для парсинга: " << pages << endl;

    for (int page = 0; page < pages; ++page) {
        respectRateLimit();
        string url = buildUrl(keyword, page);
        cout << "URL: " << url << endl;
        cout << "Загрузка страницы " << (page + 1) << "/" << pages << "... " << flush;

        string response = sendHttpRequest(url);

        if (response.empty()) {
            cout << "Ошибка" << endl;
            continue;
        }

        cout << "Получено " << response.size() << " байт" << endl;

        auto vacancies = parsePage(response, page);
        allVacancies.insert(allVacancies.end(), vacancies.begin(), vacancies.end());
        cout << "Найдено " << vacancies.size() << " вакансий" << endl;
    }

    cout << "Всего собрано вакансий: " << allVacancies.size() << endl;
    return allVacancies;
}