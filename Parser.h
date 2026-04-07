#pragma once
#include <string>
#include <vector>
#include "Vacancy.h"
using namespace std;

class Parser {
public:
    virtual ~Parser() = default;
    virtual vector<Vacancy> parse(const string& keyword) = 0;
    virtual string getParserName() const = 0;
};