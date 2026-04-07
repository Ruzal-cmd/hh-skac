#include "Vacancy.h"
#include <sstream>
using namespace std;

Vacancy::Vacancy()
    : salary_from(0), salary_to(0), has_salary_from(false), has_salary_to(false) {
}

Vacancy::Vacancy(string id, string title, string company,
    int salary_from, int salary_to,
    bool has_salary_from, bool has_salary_to,
    string description, string url, string city)
    : id(move(id)), title(move(title)), company(move(company)),
    salary_from(salary_from), salary_to(salary_to),
    has_salary_from(has_salary_from), has_salary_to(has_salary_to),
    description(move(description)), url(move(url)), city(move(city)) {
}

int Vacancy::getAverageSalary() const {
    if (has_salary_from && has_salary_to) {
        return (salary_from + salary_to) / 2;
    }
    else if (has_salary_from) {
        return salary_from;
    }
    else if (has_salary_to) {
        return salary_to;
    }
    return 0;
}

bool Vacancy::hasSalary() const {
    return has_salary_from || has_salary_to;
}

string Vacancy::getSalaryString() const {
    if (!hasSalary()) return "Не указана";
    stringstream ss;
    if (has_salary_from) ss << "от " << salary_from;
    if (has_salary_to) {
        if (has_salary_from) ss << " ";
        ss << "до " << salary_to;
    }
    ss << " руб.";
    return ss.str();
}

string Vacancy::toCSV() const {
    stringstream ss;
    ss << "\"" << id << "\","
        << "\"" << title << "\","
        << "\"" << company << "\","
        << "\"" << getSalaryString() << "\","
        << "\"" << getAverageSalary() << "\","
        << "\"" << city << "\","
        << "\"" << url << "\"";
    return ss.str();
}

string Vacancy::getCSVHeader() {
    return "ID,Название,Компания,Зарплата,Средняя зарплата,Город,Ссылка";
}