#include "HHParser.h"
#include "ParserManager.h"
#include <iostream>
#include <limits>
#include <memory>
#include <clocale>
#include <windows.h>
using namespace std;

void printMenu() {
    cout << "\nПАРСЕР ВАКАНСИЙ HH.RU" << endl;
    cout << "1. Поиск вакансий" << endl;
    cout << "2. Показать статистику" << endl;
    cout << "3. Сохранить в CSV" << endl;
    cout << "4. Показать вакансии" << endl;
    cout << "5. Очистить результаты" << endl;
    cout << "0. Выход" << endl;
    cout << "Выбор: ";
}

int main() {
    // Настройка кодировки для UTF-8
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");
    system("chcp 65001 > nul"); 

    ParserManager manager;
    string lastKeyword;

    cout << "ПАРСЕР ВАКАНСИЙ HH.RU - C++17" << endl;
    cout << "Используется RapidJSON + WinHTTP" << endl;

    manager.addParser(make_unique<HHParser>(3));

    int choice;
    do {
        printMenu();
        cin >> choice;

        switch (choice) {
        case 1: {
            cout << "Введите ключевое слово для поиска: ";
            cin.ignore();
            getline(cin, lastKeyword);
            if (lastKeyword.empty()) {
                cout << "Ключевое слово не может быть пустым!" << endl;
                break;
            }
            manager.clear();
            manager.parseAll(lastKeyword);
            if (manager.getVacancyCount() > 0) {
                cout << "\nУспешно спарсено " << manager.getVacancyCount() << " вакансий!" << endl;
                manager.displayStatistics();
            }
            else {
                cout << "Вакансий не найдено по запросу: " << lastKeyword << endl;
            }
            break;
        }
        case 2:
            if (manager.getVacancyCount() > 0) {
                manager.displayStatistics();
            }
            else {
                cout << "Нет данных. Сначала выполните поиск вакансий (пункт 1)." << endl;
            }
            break;
        case 3:
            if (manager.getVacancyCount() > 0) {
                system("mkdir results 2>nul");
                string filename = "results/vacancies_" + lastKeyword + ".csv";
                if (manager.saveToCSV(filename)) {
                    cout << "Данные успешно сохранены!" << endl;
                }
            }
            else {
                cout << "Нет данных для сохранения. Сначала выполните поиск." << endl;
            }
            break;
        case 4:
            if (manager.getVacancyCount() > 0) {
                manager.displayResults();
            }
            else {
                cout << "Нет данных. Сначала выполните поиск вакансий (пункт 1)." << endl;
            }
            break;
        case 5:
            manager.clear();
            cout << "Результаты очищены." << endl;
            break;
        case 0:
            cout << "До свидания!" << endl;
            break;
        default:
            cout << "Неверный выбор. Попробуйте снова." << endl;
        }
    } while (choice != 0);

    return 0;
}