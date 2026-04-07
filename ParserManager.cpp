#include "ParserManager.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <iomanip>
#include <windows.h>
using namespace std;

string utf8ToWin1251(const string& utf8Str) {
    if (utf8Str.empty()) return "";
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (wlen <= 0) return utf8Str;
    wchar_t* wbuf = new wchar_t[wlen];
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wbuf, wlen);
    int len = WideCharToMultiByte(1251, 0, wbuf, -1, NULL, 0, NULL, NULL);
    char* buf = new char[len];
    WideCharToMultiByte(1251, 0, wbuf, -1, buf, len, NULL, NULL);
    string result(buf);
    delete[] wbuf;
    delete[] buf;
    return result;
}

void ParserManager::addParser(unique_ptr<Parser> parser) {
    parsers.push_back(move(parser));
}

void ParserManager::parseAll(const string& keyword) {
    allVacancies.clear();
    for (auto& parser : parsers) {
        cout << "\nИспользуется: " << parser->getParserName() << endl;
        auto vacancies = parser->parse(keyword);
        allVacancies.insert(allVacancies.end(), vacancies.begin(), vacancies.end());
    }
}

ParserManager::Statistics ParserManager::calculateStatistics() const {
    Statistics stats;
    stats.totalVacancies = allVacancies.size();
    int salarySum = 0;
    map<string, int> cityCount;

    for (const auto& vacancy : allVacancies) {
        if (vacancy.hasSalary()) {
            stats.vacanciesWithSalary++;
            salarySum += vacancy.getAverageSalary();
        }
        if (vacancy.getCity() != "Не указан" && !vacancy.getCity().empty()) {
            cityCount[vacancy.getCity()]++;
        }
    }

    if (stats.vacanciesWithSalary > 0) {
        stats.averageSalary = (double)salarySum / stats.vacanciesWithSalary;
    }

    if (!cityCount.empty()) {
        stats.mostCommonCity = max_element(cityCount.begin(), cityCount.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; })->first;
    }
    return stats;
}

void ParserManager::displayResults() const {
    if (allVacancies.empty()) {
        cout << "Нет вакансий для отображения." << endl;
        return;
    }

    cout << "\nСПИСОК ВАКАНСИЙ\n" << endl;

    for (size_t i = 0; i < min(allVacancies.size(), size_t(10)); ++i) {
        const auto& v = allVacancies[i];
        cout << (i + 1) << ". " << utf8ToWin1251(v.getTitle()) << endl;
        cout << "   Компания: " << utf8ToWin1251(v.getCompany()) << endl;
        cout << "   Зарплата: " << utf8ToWin1251(v.getSalaryString()) << endl;
        cout << "   Город: " << utf8ToWin1251(v.getCity()) << endl;
        string desc = utf8ToWin1251(v.getDescription());
        if (desc.length() > 100) desc = desc.substr(0, 100) + "...";
        cout << "   Описание: " << desc << endl;
        cout << "   Ссылка: " << v.getUrl() << endl;
        cout << "--------------------------------------------------" << endl;
    }

    if (allVacancies.size() > 10) {
        cout << "\n... и еще " << (allVacancies.size() - 10) << " вакансий." << endl;
    }
}

void ParserManager::displayStatistics() const {
    auto stats = calculateStatistics();
    cout << "\nСТАТИСТИКА\n" << endl;
    cout << "Всего найдено вакансий: " << stats.totalVacancies << endl;
    cout << "Вакансий с указанием зарплаты: " << stats.vacanciesWithSalary << endl;
    cout << "Средняя зарплата: ";
    if (stats.averageSalary > 0) {
        cout << fixed << setprecision(0) << stats.averageSalary << " руб." << endl;
    }
    else {
        cout << "Нет данных" << endl;
    }
    if (!stats.mostCommonCity.empty()) {
        cout << "Самый популярный город: " << utf8ToWin1251(stats.mostCommonCity) << endl;
    }
}

bool ParserManager::saveToCSV(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл: " << filename << endl;
        return false;
    }
    file << Vacancy::getCSVHeader() << endl;
    for (const auto& vacancy : allVacancies) {
        file << vacancy.toCSV() << endl;
    }
    file.close();
    cout << "Данные сохранены в: " << filename << endl;
    return true;
}

void ParserManager::clear() {
    allVacancies.clear();
}