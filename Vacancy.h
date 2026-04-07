#pragma once
#include <string>
using namespace std;

class Vacancy {
private:
    string id;
    string title;
    string company;
    int salary_from;
    int salary_to;
    bool has_salary_from;
    bool has_salary_to;
    string description;
    string url;
    string city;

public:
    Vacancy();
    Vacancy(string id, string title, string company,
        int salary_from, int salary_to,
        bool has_salary_from, bool has_salary_to,
        string description, string url, string city);

    string getId() const { return id; }
    string getTitle() const { return title; }
    string getCompany() const { return company; }
    int getSalaryFrom() const { return salary_from; }
    int getSalaryTo() const { return salary_to; }
    bool hasSalaryFrom() const { return has_salary_from; }
    bool hasSalaryTo() const { return has_salary_to; }
    string getDescription() const { return description; }
    string getUrl() const { return url; }
    string getCity() const { return city; }

    int getAverageSalary() const;
    bool hasSalary() const;
    string getSalaryString() const;
    string toCSV() const;
    static string getCSVHeader();
};