#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include "textaanalyzer.h"
#include <boost/locale.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cctype>
#include <locale>
#include <regex>
#include <sstream>
#include <cwctype>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include "utils.h"
#include <windows.h>
#include <boost/locale/util.hpp>

namespace std
{

    namespace fs = filesystem;

    void initLocale() {
        try {
            // Генератор локалей Boost.Locale
            boost::locale::generator gen;
            locale loc = gen("ru_RU.UTF-8");

            // Устанавливаем глобальную локаль
            locale::global(loc);

            // Конфигурация потоков ввода-вывода
            wcin.imbue(loc);
            wcout.imbue(loc);
            cin.imbue(loc);
            cout.imbue(loc);

#ifdef _WIN32
            // Настройка Windows-консоли для UTF-8
            SetConsoleCP(CP_UTF8);
            SetConsoleOutputCP(CP_UTF8);
#endif
        }
        catch (const exception& e) {
            cerr << "Ошибка инициализации локали: " << e.what() << endl;
        }
    }

    TextAnalyzer::TextAnalyzer(const AnalysisSettings &settings)
        : settings(settings)
    {
        initLocale(); // Инициализация локали при создании анализатора
    }

    bool
    TextAnalyzer::isValidWord(const wstring &word) const
    {
        bool hasAlpha = false;

        for (wchar_t c : word)
        {
            if (iswalpha(c))
            {
                hasAlpha = true;

                if (!settings.allowedAlphabet.empty() &&
                    settings.allowedAlphabet.find(c) == wstring::npos)
                {
                    return false;
                }
            }
            else  if ((!settings.getRules(L"ignoreNumbers") && iswdigit(c)) ||
                (!settings.getRules(L"ignoreSpecialChars") && !iswspace(c) && !iswalnum(c))) {
                hasAlpha = true;
            }
        }

        return hasAlpha;
    }

    void TextAnalyzer::printCurrentResults() const {
        // Заголовки таблицы
        wcout << L"┌───────────────┬───────────────┬───────────────┐\n";
        wcout << L"│     Слово     │    Частота    │     Длина     │\n";
        wcout << L"├───────────────┼───────────────┼───────────────┤\n";

        // Данные таблицы
        for (const auto& result : results) {
            for (const auto& [word, frequency] : result.wordFrequency) {
                wcout << L"│ "
                           << setw(13) << left << word << L" │ "
                           << setw(13) << left << frequency << L" │ "
                           << setw(13) << left << word.length() << L" │\n";
            }
        }

        // Завершение таблицы
        wcout << L"└───────────────┴───────────────┴───────────────┘\n";
    }
    bool
    TextAnalyzer::isStopWord(const wstring &word) const
    {
        return settings.getRules(L"ignoreStopWords") && settings.stopWords.find(word) != settings.stopWords.end();
    }

    wstring TextAnalyzer::processWord(wstring word) const {
        wstring processed;
        for (const wchar_t& c : word)
        {
            if (iswalpha(c))
                processed += settings.getRules(L"caseInsensitive") ? towlower(c) : c;
            else
            {
                if ((!settings.getRules(L"ignoreNumbers") && iswdigit(c)) ||
                    (!settings.getRules(L"ignoreSpecialChars") && !iswspace(c) && !iswalnum(c))) {
                    processed += c;
                }
            }
        }
        return processed;
    }

    vector<wstring> TextAnalyzer::readFile(const wstring& filename) {
        vector<wstring> lines;

        // Создаем конвертер для имени файла
        string narrow_name = boost::locale::conv::utf_to_utf<char>(filename);

        // Открываем файл с правильной локалью
        locale loc = locale();
        wifstream file;
        file.imbue(loc);
        std::filesystem::path filepath(filename);
        file.open(filepath);
        if (!file.is_open()) {
            wcerr << L"ОШИБКА: Не удалось открыть файл: " << filename << endl;
        }
        if (file.is_open()) {
            wstring line;
            while (getline(file, line)) {
                lines.push_back(line);
            }
            file.close();
        }
        else {
            wcerr << L"ОШИБКА: Не удалось открыть файл: " << filename << endl;
        }
        return lines;
    }

    vector<wstring>
    TextAnalyzer::split(const wstring &str, wchar_t delimiter)
    {
        vector<wstring> tokens;
        wstring token;
        wistringstream tokenStream(str);
        while (getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    void
    TextAnalyzer::analyzeFiles(const vector<wstring> &filePaths, const wstring &sourceIdentifier)
    {
        try
        {
            wcout << L"DEBUG: Очистка предыдущих результатов " << endl;
            results.clear();
            wcout << L"DEBUG: Начало анализа для " << sourceIdentifier << endl;
            wcout << L"DEBUG: Текущие настройки: " << endl;
            wcout << L"Регистронезависимость: " << (settings.getRules(L"caseInsensitive") ? L"Включено" : L"Выключено") << endl;
            wcout << L"1. Игнорирование цифр: " << (settings.getRules(L"ignoreNumbers") ? L"Включено" : L"Выключено") << endl;
            wcout << L"2. Игнорирование спецсимволов: " << (settings.getRules(L"ignoreSpecialChars") ? L"Включено" : L"Выключено") << endl;
            wcout << L"3. Разрешенный алфавит: " << settings.allowedAlphabet << endl;
            wcout << L"4. Игнорирование стоп-слов: " << (settings.getRules(L"ignoreStopWords") ? L"Включено" : L"Выключено") << endl;
            wcout << L"5. Стоп-слова: " << endl;
            for (const auto &word : settings.stopWords)
            {
                wcout << word << L" ";
            }
            wcout << endl;

            AnalysisResult currentResults;
            currentResults.sourceIdentifier = sourceIdentifier;

            for (const auto &filePath : filePaths)
            {
                // wcout << L"DEBUG: Открытие файла: " << filePath.c_str() << endl;

                auto lines = readFile(filePath);
                for (const wstring &line : lines)
                {
                    auto words = split(line, L' ');
                    for (const wstring &word : words)
                    {
                        int wordCount = 0;
                        int sentenceCount = 0;

                        if (!word.empty() && (word.back() == L'.' || word.back() == L'!' || word.back() == L'?'))
                        {
                            sentenceCount++;
                        }

                        wstring processedWord = processWord(word);
                        if (!processedWord.empty() && isValidWord(processedWord) && !isStopWord(processedWord))
                        {
                            currentResults.wordFrequency[processedWord]++;
                            currentResults.wordsByLength[processedWord.length()].insert(processedWord);
                            wordCount++;
                        }

                        currentResults.sentenceCount += sentenceCount;
                    }
                }
            }

            // wcout << L"DEBUG: Анализ завершен для " << sourceIdentifier << endl;
            // wcout << L"DEBUG: Всего уникальных слов: " << currentResults.wordFrequency.size() << endl;

            results.push_back(currentResults);
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }
    void TextAnalyzer::analyzeDirectory(const wstring& directory) {
        // Устанавливаем рабочую директорию
        settings.setWorkingDirectory(directory);

        // Получаем список файлов в директории
        vector<wstring> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().wstring());
            }
        }

        size_t totalFiles = files.size();
        size_t processedFiles = 0;

        // Анализируем каждый файл
        for (const auto& filePath : files) {
            wstring fileName = fs::path(filePath).filename().wstring();
            analyzeFiles({filePath}, fileName);

            // Обновляем прогресс-бар
            processedFiles++;
            showProgressBar(processedFiles, totalFiles);

            // Имитация задержки для наглядности (можно удалить)
             this_thread::sleep_for(chrono::milliseconds(400));
        }

        // Завершаем прогресс-бар
        wcout << L"\nАнализ завершен.\n";

        // Вывод итогов для директории
        wcout << L"\n=== Итоги анализа для директории: " << directory << L" ===\n";
        printTopWords(10, L""); // Пустой идентификатор для общих результатов
        printLongestWords(5, L"");
        printSentenceCount(L"");
    }
    void
    TextAnalyzer::processFile(const wstring &filePath, const wstring &sourceIdentifier)
    {
        wstring extension = filePath.substr(filePath.find_last_of(L'.') + 1);

        if (extension == L"txt")
        {
            analyzeFiles({filePath}, sourceIdentifier);
            // Вывод общих результатов для всех файлов
            printTopWords(10, sourceIdentifier); // Пустой идентификатор для общих результатов
            printLongestWords(5, sourceIdentifier);
            printSentenceCount(sourceIdentifier);
        }
        else
        {
            wcerr << L"Неподдерживаемый формат файла: " << filePath << endl;
        }
    }
    void
    TextAnalyzer::printTopWords(size_t count, const wstring &sourceIdentifier) const
    {
        try
        {
            map<wstring, int> combinedWordFrequency;

            // Если идентификатор пустой, объединяем результаты для всех файлов
            if (sourceIdentifier.empty())
            {
                for (const auto &result : results)
                {
                    for (const auto &[word, frequency] : result.wordFrequency)
                    {
                        combinedWordFrequency[word] += frequency;
                    }
                }
            }
            else
            {
                // Иначе ищем результаты для конкретного идентификатора
                const AnalysisResult *targetResults = nullptr;
                for (const auto &result : results)
                {
                    if (result.sourceIdentifier == sourceIdentifier)
                    {
                        targetResults = &result;
                        break;
                    }
                }

                if (!targetResults)
                {
                    wcout << L"DEBUG: Результаты не найдены для " << sourceIdentifier << endl;
                    return;
                }

                combinedWordFrequency = targetResults->wordFrequency;
            }

            // Сортировка и вывод топ-N слов
            vector<pair<wstring, int>> words(combinedWordFrequency.begin(), combinedWordFrequency.end());
            sort(words.begin(), words.end(), [](const auto &a, const auto &b)
                      { return a.second > b.second; });

            wcout << L"Топ " << count << L" самых часто встречающихся слов";
            if (!sourceIdentifier.empty())
            {
                wcout << L" для " << sourceIdentifier;
            }
            wcout << L":\n";

            for (size_t i = 0; i < min(count, words.size()); ++i)
            {
                wcout << L"  " << words[i].first << L": " << words[i].second << L" раз\n";
            }
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }

    void
    TextAnalyzer::printLongestWords(size_t count, const wstring &sourceIdentifier) const
    {
        try
        {
            map<size_t, set<wstring>> combinedWordsByLength;

            // Если идентификатор пустой, объединяем результаты для всех файлов
            if (sourceIdentifier.empty())
            {
                for (const auto &result : results)
                {
                    for (const auto &[length, words] : result.wordsByLength)
                    {
                        combinedWordsByLength[length].insert(words.begin(), words.end());
                    }
                }
            }
            else
            {
                // Иначе ищем результаты для конкретного идентификатора
                const AnalysisResult *targetResults = nullptr;
                for (const auto &result : results)
                {
                    if (result.sourceIdentifier == sourceIdentifier)
                    {
                        targetResults = &result;
                        break;
                    }
                }

                if (!targetResults)
                {
                    wcout << L"DEBUG: Результаты не найдены для " << sourceIdentifier << endl;
                    return;
                }

                combinedWordsByLength = targetResults->wordsByLength;
            }

            // Сортировка и вывод топ-N самых длинных слов
            wcout << L"Топ " << count << L" самых длинных слов";
            if (!sourceIdentifier.empty())
            {
                wcout << L" для " << sourceIdentifier;
            }
            wcout << L":\n";

            size_t printed = 0;
            for (auto it = combinedWordsByLength.rbegin(); it != combinedWordsByLength.rend() && printed < count; ++it)
            {
                for (const auto &word : it->second)
                {
                    if (printed >= count)
                        break;
                    wcout << L"  " << word << L" (длина: " << it->first << L")\n";
                    printed++;
                }
            }
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }

    void
    TextAnalyzer::printSentenceCount(const wstring &sourceIdentifier) const
    {
        try
        {
            int totalSentenceCount = 0;

            // Если идентификатор пустой, суммируем результаты для всех файлов
            if (sourceIdentifier.empty())
            {
                for (const auto &result : results)
                {
                    totalSentenceCount += result.sentenceCount;
                }
            }
            else
            {
                // Иначе ищем результаты для конкретного идентификатора
                const AnalysisResult *targetResults = nullptr;
                for (const auto &result : results)
                {
                    if (result.sourceIdentifier == sourceIdentifier)
                    {
                        targetResults = &result;
                        break;
                    }
                }

                if (!targetResults)
                {
                    wcout << L"DEBUG: Результаты не найдены для " << sourceIdentifier << endl;
                    return;
                }

                totalSentenceCount = targetResults->sentenceCount;
            }

            wcout << L"Количество предложений";
            if (!sourceIdentifier.empty())
            {
                wcout << L" для " << sourceIdentifier;
            }
            wcout << L": " << totalSentenceCount << endl;
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }

    void
    TextAnalyzer::printSettings() const
    {
        try
        {
            wcout << L"Текущие настройки:\n";
            wcout << L"Регистронезависимость: "
                       << (settings.getRules(L"caseInsensitive") ? L"Включено" : L"Выключено") << endl;
            wcout << L"1. Игнорирование цифр: "
                       << (settings.getRules(L"ignoreNumbers") ? L"Включено" : L"Выключено") << endl;
            wcout << L"2. Игнорирование спецсимволов: "
                       << (settings.getRules(L"ignoreSpecialChars") ? L"Включено" : L"Выключено") << endl;
            wcout << L"3. Разрешенный алфавит: "
                       << settings.allowedAlphabet << endl;
            wcout << L"4. Игнорирование пунктуации: "
                       << (settings.getRules(L"ignorePunctuation") ? L"Включено" : L"Выключено") << endl;
            wcout << L"5. Стоп-слова: " << endl;
            for (const auto &word : settings.stopWords)
            {
                wcout << word << " ";
            }
            wcout << "\n";
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }
    void TextAnalyzer::printMostlyAphabet() {
        wstring alphabet;
        wstring choice;

        // Предложение выбора
        std::wcout << L"Выберите действие:\n";
        std::wcout << L"1. Добавить букву(буквы, слова)\n";
        std::wcout << L"2. Заменить алфавит полностью\n";
        std::wcout << L"Ваш выбор: ";
        std::wcin >> choice;
        if (!isValidString(choice)) {
			std::wcin.clear(); // Сбросить флаги ошибок
			std::wcin.sync();   // Очистить буфер
			std::wcout << L"Неверный ввод. Пожалуйста, введите строку.\n";
			return;
        }
		int ch = std::stoi(choice);
        // Обработка выбора
        if (ch == 1)
        {
            wstring words;
            wcout<< L"Введите букву(буквы, слова): ";
            std::wcin.ignore(1000, L'\n');  // Фиксированное большое число
            std::getline(std::wcin, words);
            if (std::wcin.fail()) {
                std::wcin.clear(); // Сбросить флаги ошибок
                std::wcin.sync();   // Очистить буфер
                std::wcout << L"Неверный ввод. Пожалуйста, введите строку.\n";
                return;
            }

			if (words.empty())
			{
				std::wcerr << L"ОШИБКА: Неверный ввод. Пустая строка\n";
				return;
			}

            std::wcout << L"В алфавит добавится следующее: " << words << L"\n";

            settings.addToBackAlphabet(words);
        }
        else if (ch == 2)
        {
            wstring words;
            wcout << L"Введите новый алфавит: ";
            std::wcin.ignore(1000, L'\n');  // Фиксированное большое число
            std::getline(std::wcin, words);

            if (std::wcin.fail()) {
                std::wcin.clear(); // Сбросить флаги ошибок
                std::wcin.sync();   // Очистить буфер
                std::wcout << L"Неверный ввод. Пожалуйста, введите строку.\n";
                return;
            }

            if (words.empty())
            {
                std::wcerr << L"ОШИБКА: Неверный ввод. Пустая строка\n";
                return;
            }

            std::wcout << L"В алфавит добавится следующее: " << words << L"\n";

            settings.setNewAlphabet(words);
        }
        else
        {
            std::wcerr << L"ОШИБКА: Неверный выбор.\n";
            alphabet.clear(); // Очищаем директорию, чтобы указать на ошибку
        }
    }
    void TextAnalyzer::changeSettings() {
        try {
            bool exit = false;
            while (!exit) {
                wcout << L"Изменение настроек:\n";
                wcout << L"1. Регистронезависимость: "
                    << (settings.getRules(L"caseInsensitive") ? L"Включено" : L"Выключено") << endl;
                wcout << L"2. Игнорирование цифр: "
                    << (settings.getRules(L"ignoreNumbers") ? L"Включено" : L"Выключено") << endl;
                wcout << L"3. Игнорирование спецсимволов: "
                    << (settings.getRules(L"ignoreSpecialChars") ? L"Включено" : L"Выключено") << endl;
                wcout << L"4. Разрешенный алфавит: "
                    << settings.allowedAlphabet << endl;
                wcout << L"5. Игнорирование стоп-слов: "
                    << (settings.getRules(L"ignoreStopWords") ? L"Включено" : L"Выключено") << endl;
                wcout << L"6. Добавить стоп-слово\n";
                wcout << L"Выберите опцию (или введите 0 для выхода): ";

                wstring choice;

                // Считываем выбор пользователя
                wcin >> choice;
                if (!isValidString(choice)) {
                    std::wcin.clear(); // Сбросить флаги ошибок
                    std::wcin.sync();   // Очистить буфер
                    std::wcout << L"Неверный ввод. Пожалуйста, введите строку.\n";
                    return;
                }
                std::wcin.clear();
                int ch = std::stoi(choice);
                int sub_ch;
                switch (ch) {
                case 1:
                    wcout << L"Введите 1 для включения или 0 для выключения: ";
                    std::wcin >> choice;
                    if (!isValidString(choice)) {
                        std::wcin.clear(); // Сбросить флаги ошибок
                        std::wcin.sync();   // Очистить буфер
                        std::wcout << L"Неверный ввод. Пожалуйста, введите строку.\n";
                        return;
                    }
                    std::wcin.clear();
                    sub_ch = std::stoi(choice);
                    settings.changeRules(L"caseInsensitive", (sub_ch == 1));
                    break;
                case 2:
                    wcout << L"Введите 1 для включения или 0 для выключения: ";
                    std::wcin >> choice;
                    if (!isValidString(choice)) {
                        std::wcin.clear(); // Сбросить флаги ошибок
                        std::wcin.sync();   // Очистить буфер
                        std::wcout << L"Неверный ввод. Пожалуйста, введите строку.\n";
                        return;
                    }
                    std::wcin.clear();
                    sub_ch = std::stoi(choice);
                    settings.changeRules(L"ignoreNumbers", (sub_ch == 1));
                    break;
                case 3:
                    wcout << L"Введите 1 для включения или 0 для выключения: ";
                    std::wcin >> choice;
                    if (!isValidString(choice)) {
                        std::wcin.clear(); // Сбросить флаги ошибок
                        std::wcin.sync();   // Очистить буфер
                        std::wcout << L"Неверный ввод. Пожалуйста, введите строку.\n";
                        return;
                    }
                    std::wcin.clear();
                    sub_ch = std::stoi(choice);
                    settings.changeRules(L"ignoreSpecialChars", (sub_ch == 1));
                    break;
                case 4:
                    printMostlyAphabet();
                    break;
                case 5:
                    wcout << L"Введите 1 для включения или 0 для выключения: ";
                    std::wcin >> choice;
                    if (!isValidString(choice)) {
                        std::wcin.clear(); // Сбросить флаги ошибок
                        std::wcin.sync();   // Очистить буфер
                        std::wcout << L"Неверный ввод. Пожалуйста, введите строку.\n";
                        return;
                    }
                    std::wcin.clear();
                    sub_ch = std::stoi(choice);
                    settings.changeRules(L"ignoreStopWords", (sub_ch == 1));
                    break;
                case 6: {
                    std::wcin.clear(); // Сбросить флаги ошибок
                    std::wcin.ignore(1000000, L'\n'); // Игнорировать до 1 000 000 символов или до '\n'
                    wcout << L"Введите новое стоп-слово: ";
                    wstring stopWord;
                    std::getline(std::wcin, stopWord);

                    if (std::wcin.fail()) {
                        std::wcin.clear(); // Сбросить флаги ошибок
                        std::wcin.sync();  // Очистить буфер
                        std::wcout << L"Неверный ввод. Пожалуйста, введите строку.\n";
                        continue;
                    }

                    if (stopWord.empty()) {
                        wcout << L"Пусто (введите корректное стоп-слово).\n";
                        break;
                    }

                    settings.stopWords.insert(stopWord);
                    wcout << L"Стоп-слово добавлено: " << stopWord << L"\n";
                    break;
                }
                case 0:
                    exit = true;
                    break;
                default:
                    wcout << L"Неверный выбор. Попробуйте снова.\n";
                }
            }
        }
        catch (const exception& e) {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }


    void
    TextAnalyzer::clearResults()
    {
        try
        {
            results.clear();
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }

        void TextAnalyzer::saveResults(const std::wstring& filename) const {
        try {
            // Создаем файловый поток
            std::wofstream file;

            // Создаем генератор локали Boost
            boost::locale::generator gen;
            std::locale utf8_locale = gen("en_US.UTF-8");

            // Устанавливаем локаль для файла
            file.imbue(utf8_locale);

            // Открываем файл
            file.open(filename);

            if (!file.is_open()) {
                std::wcerr << L"ОШИБКА: Не удалось открыть файл для записи: " << filename << std::endl;
                return;
            }

            // Записываем данные через Boost.Locale
            file << boost::locale::translate(L"=== Результаты анализа ===").str() << std::endl;

            // Если результатов нет, выводим сообщение
            if (results.empty()) {
                file << L"Нет данных для сохранения.\n";
                file.close();
                std::wcout << L"Результаты отсутствуют. Файл сохранен без данных." << std::endl;
                return;
            }

            // Сохраняем результаты для каждого источника
            for (const auto& result : results) {
                std::wstring sourceIdentifierWstr = boost::locale::conv::utf_to_utf<wchar_t>(result.sourceIdentifier);

                file << L"\n=== Результаты для источника: " << sourceIdentifierWstr << L" ===\n";

                // Топ 10 самых часто встречающихся слов
                file << L"Топ 10 самых часто встречающихся слов:\n";
                printTopWordsToFile(10, result.sourceIdentifier, file);

                // Топ 5 самых длинных слов
                file << L"\nТоп 5 самых длинных слов:\n";
                printLongestWordsToFile(5, result.sourceIdentifier, file);

                // Количество предложений
                file << L"\nКоличество предложений: ";
                printSentenceCountToFile(result.sourceIdentifier, file);
            }

            file.close();
            std::wcout << L"Результаты успешно сохранены в файл: " << filename << std::endl;
        }
        catch (const std::exception& e) {
            std::wcerr << L"ОШИБКА: " << e.what() << std::endl;
        }
    }


    void TextAnalyzer::printTopWordsToFile(size_t count, const wstring &sourceIdentifier, wofstream &file) const {
        try {
            // Поиск результатов для указанного источника
            const AnalysisResult *targetResults = nullptr;
            for (const auto &result : results) {
                if (result.sourceIdentifier == sourceIdentifier) {
                    targetResults = &result;
                    break;
                }
            }

            // Если результаты не найдены, вывести сообщение
            if (!targetResults) {
                file << L"DEBUG: Результаты не найдены для "
                    << boost::locale::conv::utf_to_utf<wchar_t>(sourceIdentifier) // Убедитесь, что поток поддерживает широкие строки
                    << L'\n';
                return;
            }

            // Преобразование словаря частот в вектор пар
            vector<pair<wstring, int>> words(targetResults->wordFrequency.begin(), targetResults->wordFrequency.end());

            // Сортировка по убыванию частоты
            sort(words.begin(), words.end(), [](const auto &a, const auto &b) {
                return a.second > b.second;
            });

            // Запись топ-N слов в файл
            for (size_t i = 0; i < min(count, words.size()); ++i) {
                file << L"  " << words[i].first << L": " << words[i].second << L" раз\n";
            }
        } catch (const exception &e) {
            wcerr << L"ОШИБКА: " << boost::locale::conv::utf_to_utf<wchar_t>(e.what()) << L'\n';
        }
    }

    void TextAnalyzer::printLongestWordsToFile(size_t count, const wstring &sourceIdentifier, wofstream &file) const {
        try {
            // Поиск результатов для указанного источника
            const AnalysisResult *targetResults = nullptr;
            for (const auto &result : results) {
                if (result.sourceIdentifier == sourceIdentifier) {
                    targetResults = &result;
                    break;
                }
            }

            // Если результаты не найдены, вывести сообщение
            if (!targetResults) {
                file << L"DEBUG: Результаты не найдены для " 
                    << boost::locale::conv::utf_to_utf<wchar_t>(sourceIdentifier) 
                    << L'\n';
                return;
            }

            // Переменная для отслеживания количества напечатанных слов
            size_t printed = 0;

            // Итерация по словам в порядке убывания длины
            for (auto it = targetResults->wordsByLength.rbegin(); 
                it != targetResults->wordsByLength.rend() && printed < count; 
                ++it) {
                for (const auto &word : it->second) {
                    if (printed >= count) break;

                    file << L"  " << word 
                        << L" (длина: " << it->first << L")\n";
                    printed++;
                }
            }
        } catch (const exception &e) {
            wcerr << L"ОШИБКА: " 
                << boost::locale::conv::utf_to_utf<wchar_t>(e.what()) 
                << L'\n';
        }
    }


    void TextAnalyzer::printSentenceCountToFile(const wstring &sourceIdentifier, wofstream &file) const
    {
        try
        {
            const AnalysisResult *targetResults = nullptr;
            for (const auto &result : results)
            {
                if (result.sourceIdentifier == sourceIdentifier)
                {
                    targetResults = &result;
                    break;
                }
            }
            if (!targetResults) {
                file << L"DEBUG: Результаты не найдены для " 
                    << boost::locale::conv::utf_to_utf<wchar_t>(sourceIdentifier) 
                    << L'\n';
                return;
            }
            file << targetResults->sentenceCount << endl;
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }

    void TextAnalyzer::loadResults(const wstring& filename) {
        try {

            wifstream file;
            // Создаем генератор локали Boost
            boost::locale::generator gen;
            std::locale utf8_locale = gen("en_US.UTF-8");

            // Устанавливаем локаль для файла
            file.imbue(utf8_locale);

            // Открываем файл
            file.open(filename);

            if (!file.is_open()) {
                wcerr << L"ОШИБКА: Не удалось открыть файл для чтения: " << filename << endl;
                return;
            }

            // Очистка текущих результатов
            results.clear();

            wstring line;
            AnalysisResult currentResults;
            wstring currentSource;

            while (getline(file, line)) {
                wcout << L"Прочитанная строка: [" << line << L"]" << endl;
// Начало нового блока результатов
                if (line.find(L"=== Результаты для источника: ") != wstring::npos) {
                    if (!currentSource.empty()) {
                        results.push_back(currentResults);
                        currentResults = AnalysisResult(); // Сброс текущих результатов
                    }

                    // Извлечение имени источника
                    size_t start = line.find(L": ") + 2;
                    size_t end = line.find(L" ===");
                    currentSource = line.substr(start, end - start);
                    currentResults.sourceIdentifier = currentSource;
                } else if (line.find(L"Топ") != wstring::npos) {
                    // Обработка топовых слов
                    wregex regex(L"Топ (\\d+) самых часто встречающихся слов для ([-а-яА-ЯёЁa-zA-Z]+):");
                    wsmatch match;
                    if (regex_search(line, match, regex)) {
                        currentSource = match[2];
                        currentResults.sourceIdentifier = currentSource;
                    }
                } else if (line.find(L": ") != wstring::npos) {
                    // Обработка строк с частотами слов
                    size_t colonPos = line.find(L": ");
                    wstring word = line.substr(0, colonPos);
                    int count = stoi(line.substr(colonPos + 2));
                    currentResults.wordFrequency[word] = count;
                } else if (line.find(L"Количество предложений для") != wstring::npos) {
                    // Обработка количества предложений
                    wregex regex(L"Количество предложений для ([-а-яА-ЯёЁa-zA-Z]+): (\\d+)");
                    wsmatch match;
                    if (regex_search(line, match, regex)) {
                        currentResults.sentenceCount = stoi(match[2]);
                    }
                }
            }

            // Добавление последнего блока результатов
            if (!currentSource.empty()) {
                results.push_back(currentResults);
            }

            file.close();
            wcout << L"Результаты успешно загружены из файла: " << filename << endl;
        } catch (const exception& e) {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }
    TextAnalyzer::result_vector TextAnalyzer::loadExternalResult(const wstring& filename) {
        try{
            wifstream file;
            // Создаем генератор локали Boost
            boost::locale::generator gen;
            std::locale utf8_locale = gen("en_US.UTF-8");

            // Устанавливаем локаль для файла
            file.imbue(utf8_locale);

            // Открываем файл
            file.open(filename);

            if (!file.is_open()) {
                wcerr << L"ОШИБКА: Не удалось открыть файл для чтения: " << filename << endl;
                return {};
            }

            result_vector result;

            wstring line;
            AnalysisResult currentResults;
            wstring currentSource;

            while (getline(file, line)) {
                wcout << L"Прочитанная строка: [" << line << L"]" << endl;

                // Начало нового блока результатов
                if (line.find(L"=== Результаты для источника: ") != wstring::npos) {
                    if (!currentSource.empty()) {
                        result.push_back(currentResults);
                        currentResults = AnalysisResult(); // Сброс текущих результатов
                    }

                    // Извлечение имени источника
                    size_t start = line.find(L": ") + 2;
                    size_t end = line.find(L" ===");
                    currentSource = line.substr(start, end - start);
                    currentResults.sourceIdentifier = currentSource;
                }
                // Обработка топовых слов (частот)
                else if (line.find(L"Топ") != wstring::npos && line.find(L"самых часто встречающихся слов:") != wstring::npos) {
                    // Пропускаем строку с заголовком и начинаем читать слова
                    while (getline(file, line)) {
                        if (line.empty()) {
                            break; // Завершаем, если достигли пустой строки
                        }
                        // Обработка строки с частотой слова
                        wregex wordRegex(L"  (.+?): (\\d+) раз");
                        wsmatch wordMatch;
                        if (regex_search(line, wordMatch, wordRegex)) {
                            wstring word = wordMatch[1]; // Само слово
                            int count = stoi(wordMatch[2]); // Частота
                            currentResults.wordFrequency[word] = count;
                        }
                    }
                }
                // Обработка топовых слов (длина)
                else if (line.find(L"Топ") != wstring::npos && line.find(L"самых длинных слов:") != wstring::npos) {
                    // Пропускаем строку с заголовком и начинаем читать слова
                    while (getline(file, line)) {
                        if (line.empty()) {
                            break; // Завершаем, если достигли пустой строки
                        }
                        // Обработка строки с длинным словом
                        wregex wordRegex(L"  (.+?) \\(длина: (\\d+)\\)");
                        wsmatch wordMatch;
                        if (regex_search(line, wordMatch, wordRegex)) {
                            wstring word = wordMatch[1]; // Само слово
                            size_t length = stoi(wordMatch[2]); // Длина слова
                            currentResults.wordsByLength[length].insert(word); // Добавляем в map
                        }
                    }
                }
                // Обработка количества предложений
                else if (line.find(L"Количество предложений:") != wstring::npos) {
                    wregex regex(L"Количество предложений: (\\d+)");
                    wsmatch match;
                    if (regex_search(line, match, regex)) {
                        currentResults.sentenceCount = stoi(match[1]);
                    }
                }
            }

            // Добавление последнего блока результатов
            if (!currentSource.empty()) {
                result.push_back(currentResults);
            }

            file.close();
            wcout << L"Результаты успешно загружены из файла: " << filename << endl;
            return result;
        }
        catch (const exception& e) {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }
    // Функция для парсинга строки с длинными словами
    void TextAnalyzer::parseLongestWords(const wstring& line, map<size_t, set<wstring>>& wordsByLength) {
        size_t length;
        wstring word;
        wstringstream ss(line);
        ss >> word >> length;
        wordsByLength[length].insert(word);
    }

    // Функция для парсинга строки с частотой слов
    void TextAnalyzer::parseWordFrequency(const wstring& line, map<wstring, int>& wordFrequency) {
        wstringstream ss(line);
        wstring word;
        int count;
        wchar_t colon;
        ss >> word >> colon >> count;
        wordFrequency[word] = count;
    }

    // Функция для парсинга файла с результатами анализа
    TextAnalyzer::AnalysisResult TextAnalyzer::parseAnalysisResult(const wstring& filename) {
        AnalysisResult result;
        wifstream file;
        // Создаем генератор локали Boost
        boost::locale::generator gen;
        std::locale utf8_locale = gen("en_US.UTF-8");

        // Устанавливаем локаль для файла
        file.imbue(utf8_locale);

        // Открываем файл
        file.open(filename);

        wstring line;

        while (getline(file, line)) {
            if (line.find(L"=== Результаты для источника:") != wstring::npos) {
                result.sourceIdentifier = line.substr(line.find(L":") + 2);
            }
            else if (line.find(L"Количество предложений:") != wstring::npos) {
                result.sentenceCount = stoi(line.substr(line.find(L":") + 2));
            }
            else if (line.find(L"раз") != wstring::npos) {
                parseWordFrequency(line, result.wordFrequency);
            }
            else if (line.find(L"(длина:") != wstring::npos) {
                parseLongestWords(line, result.wordsByLength);
            }
        }

        return result;
    }
    void TextAnalyzer::compareResults(const wstring &sourceIdentifier1, const wstring &sourceIdentifier2)
    {
        try
        {
            auto res_line1 = loadExternalResult(sourceIdentifier1);
            if (res_line1.size() == 0) {
                std::wcout << "Не удалось прочитать данные" << std::endl;
                return;
            }
            auto res_line2 = loadExternalResult(sourceIdentifier2);
            if (res_line2.size() == 0) {
                std::wcout << "Не удалось прочитать данные" << std::endl;
                return;
            }
            for (size_t i = 0, j = 0; i < res_line1.size(); i++, j++) {
                // Сравнение количества предложений
                wcout << L"\n=== Сравнение количества предложений ===\n";
                wcout << res_line1[i].sourceIdentifier << L": " << res_line1[i].sentenceCount << L" предложений\n";
                wcout << res_line2[i].sourceIdentifier << L": " << res_line2[i].sentenceCount << L" предложений\n";
                wcout << L"Разница: " << abs(res_line1[i].sentenceCount - res_line2[i].sentenceCount) << L" предложений\n";

                // Сравнение частот слов
                wcout << L"\n=== Сравнение частот слов ===\n";
                map<wstring, int> commonWords;
                for (const auto& [word, count] : res_line1[i].wordFrequency) {
                    if (res_line2[i].wordFrequency.find(word) != res_line2[i].wordFrequency.end()) {
                        commonWords[word] = abs(count - res_line2[i].wordFrequency.at(word));
                    }
                }

                if (!commonWords.empty()) {
                    wcout << L"Слова с наибольшей разницей в частоте:\n";
                    vector<pair<wstring, int>> sortedCommonWords(commonWords.begin(), commonWords.end());
                    sort(sortedCommonWords.begin(), sortedCommonWords.end(),
                        [](const auto& a, const auto& b) { return a.second > b.second; });

                    for (size_t i = 0; i < min(sortedCommonWords.size(), size_t(5)); ++i) {
                        wcout << L"  " << sortedCommonWords[i].first << L": разница " << sortedCommonWords[i].second << L"\n";
                    }
                }
                else {
                    wcout << L"Нет общих слов для сравнения.\n";
                }

                // Сравнение самых длинных слов
                wcout << L"\n=== Сравнение самых длинных слов ===\n";
                auto longestWords1 = res_line1[i].wordsByLength.rbegin()->second;
                auto longestWords2 = res_line2[i].wordsByLength.rbegin()->second;

                wcout << res_line1[i].sourceIdentifier << L": ";
                for (const auto& word : longestWords1) {
                    wcout << word << L" ";
                }
                wcout << L"\n";

                wcout << res_line2[i].sourceIdentifier << L": ";
                for (const auto& word : longestWords2) {
                    wcout << word << L" ";
                }
                wcout << L"\n";
            }
        }
        catch (const exception& e) {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }
}