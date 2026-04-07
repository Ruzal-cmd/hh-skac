#pragma once
#include "Parser.h"
#include "Vacancy.h"
#include <vector>
#include <memory>
#include <string>
using namespace std;

class ParserManager {
private:
    vector<unique_ptr<Parser>> parsers;
    vector<Vacancy> allVacancies;

    struct Statistics {
        int totalVacancies = 0;
        double averageSalary = 0.0;
        int vacanciesWithSalary = 0;
        string mostCommonCity;
    };

    Statistics calculateStatistics() const;

public:
    void addParser(unique_ptr<Parser> parser);
    void parseAll(const string& keyword);
    bool saveToCSV(const string& filename) const;
    void displayResults() const;
    void displayStatistics() const;
    void clear();
    const vector<Vacancy>& getVacancies() const { return allVacancies; }
    size_t getVacancyCount() const { return allVacancies.size(); }
};