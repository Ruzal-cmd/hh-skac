#pragma once
#include "Parser.h"
using namespace std;

class HHParser : public Parser {
private:
    static constexpr const char* BASE_URL = "https://api.hh.ru/vacancies";  // ÏĞÀÂÈËÜÍÛÉ URL
    static constexpr int DEFAULT_PAGES = 3;
    static constexpr int VACANCIES_PER_PAGE = 20;
    static constexpr int REQUEST_TIMEOUT_MS = 5000;
    int pages;

    string buildUrl(const string& keyword, int page) const;
    string sendHttpRequest(const string& url) const;
    vector<Vacancy> parsePage(const string& jsonData, int page);
    void respectRateLimit() const;

public:
    HHParser(int pages = DEFAULT_PAGES);
    vector<Vacancy> parse(const string& keyword) override;
    string getParserName() const override { return "HH.ru Ïàğñåğ"; }
    void setPages(int pagesCount) { pages = pagesCount; }
    int getPages() const { return pages; }
};