#include "include/core.h"
#include <iostream>
#include <limits>
#include <vector>
#include <string>
#include <map>
#include <codecvt>
#include <locale>
#include <filesystem>


#ifdef _WIN32
#include <windows.h>
#include <conio.h>

void initConsole()
{
    SetConsoleOutputCP(CP_UTF8); // Включение поддержки UTF-8
    SetConsoleCP(CP_UTF8);
}

#else
#include <unistd.h>
#include <ncurses.h>

void initConsole()
{
    setlocale(LC_ALL, "en_US.utf8"); // Включение поддержки UTF-8
}
#endif

using namespace std;
namespace fs = std::filesystem;

// Очистка консоли
void clearConsole()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Установка цвета текста (Windows)
#ifdef _WIN32
void setColor(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor()
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // Сброс к стандартному цвету
}

void setBackgroundColor(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color << 4); // Сдвиг на 4 бита для фона
}

void setConsoleBackgroundColor(int color)
{
    // Получаем хэндл консоли
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Устанавливаем цвет фона
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    WORD originalAttributes = csbi.wAttributes;

    // Заполняем всю консоль пробелами с нужным цветом фона
    COORD topLeft = {0, 0};
    DWORD consoleSize = csbi.dwSize.X * csbi.dwSize.Y;
    DWORD written;
    FillConsoleOutputCharacter(hConsole, ' ', consoleSize, topLeft, &written);
    FillConsoleOutputAttribute(hConsole, color << 4, consoleSize, topLeft, &written);

    // Устанавливаем курсор в начальную позицию
    SetConsoleCursorPosition(hConsole, topLeft);
}
#endif

// Установка цвета текста (Unix)
#ifndef _WIN32
void setColor(const std::wstring &colorCode)
{
    std::wcout << colorCode;
}

void resetColor()
{
    std::wcout << "\033[0m"; // Сброс цвета
}

void setBackgroundColor(const std::wstring &colorCode)
{
    std::wcout << colorCode;
}

void setConsoleBackgroundColor(int color)
{
    // Устанавливаем цвет фона всей консоли
    std::wcout << L"\033[48;5;" << color << L"m";

    // Очищаем консоль
    std::wcout << L"\033[2J";

    // Устанавливаем курсор в начальную позицию
    std::wcout << L"\033[H";
}
#endif

// Рисование рамки
void drawBox(const std::wstring &title)
{
    std::wcout << L"╔══════════════════════════════╗\n";
    std::wcout << L"║ " << title;
    for (size_t i = 0; i < 28 - title.size(); ++i)
        std::wcout << L" ";
    std::wcout << L" ║\n";
    std::wcout << L"╠══════════════════════════════╣\n";
}

// Основное меню
void showMenu()
{
    clearConsole();

#ifdef _WIN32
    setConsoleBackgroundColor(1); // Синий фон (Windows)
#else
    setConsoleBackgroundColor(21); // Синий фон (Unix)
#endif

    drawBox(L"★ Главное меню ★");

    std::wcout << L"║ 1. Анализировать файлы     ║\n";
    std::wcout << L"║ 2. Изменить настройки      ║\n";
    std::wcout << L"║ 3. Показать настройки      ║\n";
    std::wcout << L"║ 4. Повторный анализ файлов ║\n";
    std::wcout << L"║ 5. Сохранить результаты    ║\n";
    std::wcout << L"║ 6. Загрузить результаты    ║\n";
    std::wcout << L"║ 7. Сравнительный анализ    ║\n";
    std::wcout << L"║ 8. Работа с директорией    ║\n";
    std::wcout << L"║ 9. Показать результат      ║\n";
    std::wcout << L"║ 0. Выйти                   ║\n";
    std::wcout << L"╚══════════════════════════════╝\n";

#ifndef _WIN32
    resetColor(); // Сброс цвета (Unix)
#endif
}

// Функция для выбора директории
wstring chooseDirectory(const TextAnalyzer &analyzer)
{
    wstring directory;
    int choice;

    // Предложение выбора
    std::wcout << L"Выберите действие:\n";
    std::wcout << L"1. Использовать текущую директорию\n";
    std::wcout << L"2. Указать новую директорию\n";
    std::wcout << L"Ваш выбор: ";
    std::wcin >> choice;
    std::wcin.ignore(); // Игнорируем оставшийся символ новой строки

    // Обработка выбора
    if (choice == 1)
    {
        directory = analyzer.getCurrentDirectory();
        std::wcout << L"Используется текущая директория: " << directory << L"\n";
    }
    else if (choice == 2)
    {
        // Указать новую директорию
        std::wcout << L"Введите путь к директории: ";
        std::getline(std::wcin, directory);

        // Проверка существования директории
        if (!std::filesystem::exists(directory))
        {
            std::wcerr << L"ОШИБКА: Директория не существует.\n";
            directory.clear(); // Очищаем директорию, чтобы указать на ошибку
        }
    }
    else
    {
        std::wcerr << L"ОШИБКА: Неверный выбор.\n";
        directory.clear(); // Очищаем директорию, чтобы указать на ошибку
    }

    return directory;
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // Включение поддержки UTF-8 на Windows
#else
    setlocale(LC_ALL, "en_US.utf8"); // Включение поддержки UTF-8 на Unix
#endif

    clearConsole(); // Очистка консоли при запуске

    // Настройки анализатора
    AnalysisSettings settings;
    settings.changeRules(L"caseInsensitive", false);
    settings.changeRules(L"ignoreNumbers", false);
    settings.changeRules(L"ignoreSpecialChars", false);
    settings.changeRules(L"ignoreStopWords", false);
    settings.allowedAlphabet = L"абвгдеёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗИКЛМНОПРСТУФХЦЧШЩЪЫЬЮЯ";
    settings.stopWords = {L"и", L"в", L"не", L"на", L"с"};
    settings.workingDirectory = L"../data/";
    TextAnalyzer analyzer(settings);

    int choice;
    while (true)
    {
        showMenu();
        try
        {
            std::wcout << L"Выберите опцию: ";
            if (!(std::wcin >> choice))
            {
                std::wcin.clear();
                std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
                std::wcout << L"Неверный ввод. Пожалуйста, введите число.\n";
                continue;
            }

            std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
        }
        catch (const std::exception &e)
        {
            std::wcout << L"Неверный ввод. Пожалуйста, введите число.\n";
            continue;
        }

        switch (choice)
        {
        case 1:
        {
            std::wstring filePath;
            try
            {
                std::wcout << L"Введите путь к файлу: ";
                if (!(std::wcin >> filePath))
                {
                    std::wcin.clear();
                    std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
                    std::wcout << L"Неверный ввод. Пожалуйста, введите число.\n";
                    continue;
                }

                std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
            }
            catch (const std::exception &e)
            {
                std::wcout << L"Неверный ввод. Пожалуйста, введите число.\n";
                continue;
            }
            // Проверка существования файла
            if (!std::filesystem::exists(filePath))
            {
                std::wcerr << L"ОШИБКА: Файл не существует: " << filePath << L"\n";
                break; // Выход из case, если файл не существует
            }

            // Извлечение имени файла (без расширения) для использования как идентификатор
            std::filesystem::path path(filePath);
            std::wstring sourceIdentifier = path.stem().wstring(); // Имя файла без расширения
            // Обработка файла
            analyzer.processFile(filePath, sourceIdentifier);

            break;
        }
        case 2:
            analyzer.changeSettings();
            break;
        case 3:
            analyzer.printSettings();
            break;
        case 4:
            analyzer.clearResults();
            break;
        case 5:
        {
            std::wstring filename;
            std::wcout << L"Введите имя файла для сохранения результатов: ";
            std::getline(std::wcin, filename);

            analyzer.saveResults(filename);
            break;
        }
        case 6:
        {
            std::wstring filename;
            try{
            std::wcout << L"Введите имя файла для загрузки результатов: ";
            std::getline(std::wcin, filename);
            }catch(const std::exception &e){
                std::wcout << L"Неверный ввод. Пожалуйста, введите число.\n";
                continue;

            }
            analyzer.loadResults(filename);
            break;
        }
        case 7:
        {
            std::wstring sourceIdentifier1, sourceIdentifier2;
            try
            {

                std::wcout << L"Введите первый идентификатор источника: ";
                std::getline(std::wcin, sourceIdentifier1);
                std::wcout << L"Введите второй идентификатор источника: ";
                std::getline(std::wcin, sourceIdentifier2);
            }
            catch (const std::exception &e)
            {
                std::wcout << L"Неверный ввод. Пожалуйста, введите число.\n";
                continue;
            }
            analyzer.compareResults(sourceIdentifier1, sourceIdentifier2);
            break;
        }
        case 8:
        {
            wstring directory = chooseDirectory(analyzer);
            analyzer.analyzeDirectory(directory);

            std::wstring filename;
            try{
            std::wcout << L"Введите имя файла для сохранения результатов: ";
            if (!(std::wcin >> filename))
            {
                std::wcin.clear();
                std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
                std::wcout << L"Неверный ввод. Пожалуйста, введите число.\n";
                continue;
            }

            std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
            }catch(const std::exception &e){
                std::wcout << L"Неверный ввод. Пожалуйста, введите число.\n";
                continue;

            }
            analyzer.saveResults(filename);
            break;
        }
        case 9:
            analyzer.printCurrentResults();
            break;
        case 0:
            std::wcout << L"Выход из программы.\n";
            return 0;
        default:
            std::wcout << L"Неверный выбор. Попробуйте снова.\n";
        }

#ifdef _WIN32
        system("pause"); // Пауза для Windows
#else
        std::wcout << L"Нажмите Enter для продолжения...\n";
        std::wcin.get(); // Пауза для Unix
#endif
    }

    // Удаление тестовых файлов
#ifdef _WIN32
    std::remove("Война_и_мир.txt");
    std::remove("Анна_Каренина.txt");
    std::remove("Лолита.txt");
    std::remove("Дар.txt");
#else
    std::filesystem::remove("Война_и_мир.txt");
    std::filesystem::remove("Анна_Каренина.txt");
    std::filesystem::remove("Лолита.txt");
    std::filesystem::remove("Дар.txt");
#endif

    return 0;
}