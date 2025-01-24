#include "../include/textaanalyzer.h"

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
#include <codecvt>
#include <filesystem>
#include <thread>
#include "../include/utils.h"
//#include "../lib/tabulate.hpp" // TODO ИБИЛИОТЕКА ДЛЯ ПОСТРОЕНИЕ ТАБЛИЦ, ОНА СДЕЛАЕТ ПОКРАСИВЕЕ !!
#include <iomanip> // Для std::s etw и std::left

namespace std
{

    namespace fs = filesystem;
    TextAnalyzer::TextAnalyzer(const AnalysisSettings &settings)
        : settings(settings)
    {
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
            else if (iswdigit(c) && !iswspace(c))
            {
                hasAlpha = !settings.getRules(L"ignoreNumbers") && !settings.getRules(L"ignoreSpecialChars");
            }
            else
            {
                return false;
            }
        }

        return hasAlpha;
    }

    void TextAnalyzer::printCurrentResults() const {
        // Заголовки таблицы
        std::wcout << L"┌───────────────┬───────────────┬───────────────┐\n";
        std::wcout << L"│     Слово     │    Частота    │     Длина     │\n";
        std::wcout << L"├───────────────┼───────────────┼───────────────┤\n";

        // Данные таблицы
        for (const auto& result : results) {
            for (const auto& [word, frequency] : result.wordFrequency) {
                std::wcout << L"│ "
                           << std::setw(13) << std::left << word << L" │ "
                           << std::setw(13) << std::left << frequency << L" │ "
                           << std::setw(13) << std::left << word.length() << L" │\n";
            }
        }

        // Завершение таблицы
        std::wcout << L"└───────────────┴───────────────┴───────────────┘\n";
    }
    bool
    TextAnalyzer::isStopWord(const wstring &word) const
    {
        return settings.getRules(L"ignoreStopWords") && settings.stopWords.find(word) != settings.stopWords.end();
    }

    wstring
    TextAnalyzer::processWord(wstring word) const
    {
        wstring processed;
        for (const wchar_t &c : word)
        {
            if (iswalpha(c))
                processed += settings.getRules(L"caseInsensitive") ? towlower(c) : c;
            else
            {
                if ((!settings.getRules(L"ignoreNumbers") && iswdigit(c)) ||
                    (!settings.getRules(L"ignoreSpecialChars") && !iswspace(c)))
                    processed += c;
            }
        }
        return processed;
    }

    vector<wstring>
    TextAnalyzer::readFile(const wstring &filename)
    {
        ifstream file;

        // Преобразование имени файла для разных ОС
#ifdef _WIN32
        // Для Windows: используем широкие символы напрямую
        file.open(filename.c_str());
#else
        // Для Unix: конвертируем wstring в UTF-8 string
        wstring_convert<codecvt_utf8<wchar_t>> converter;
        string narrowName = converter.to_bytes(filename);
        file.open(narrowName);
#endif

        vector<wstring> lines;
        string line;
        if (file.is_open())
        {
            while (getline(file, line))
            {
                wstring wline = convertToWstring(line);
                lines.push_back(wline);
            }
            file.close();
        }
        else
        {
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
    void TextAnalyzer::analyzeDirectory(const std::wstring& directory) {
        // Устанавливаем рабочую директорию
        settings.setWorkingDirectory(directory);

        // Получаем список файлов в директории
        std::vector<std::wstring> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().wstring());
            }
        }

        size_t totalFiles = files.size();
        size_t processedFiles = 0;

        // Анализируем каждый файл
        for (const auto& filePath : files) {
            std::wstring fileName = fs::path(filePath).filename().wstring();
            analyzeFiles({filePath}, fileName);

            // Обновляем прогресс-бар
            processedFiles++;
            showProgressBar(processedFiles, totalFiles);

            // Имитация задержки для наглядности (можно удалить)
             std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }

        // Завершаем прогресс-бар
        std::wcout << L"\nАнализ завершен.\n";

        // Вывод итогов для директории
        std::wcout << L"\n=== Итоги анализа для директории: " << directory << L" ===\n";
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

    void
    TextAnalyzer::changeSettings()
    {
        try
        {
            bool exit = false;
            while (!exit)
            {
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

                int choice;
                wcin >> choice;
                wcin.ignore(numeric_limits<streamsize>::max(), L'\n'); // Очистка буфера ввода
                // Проверка на некорректный ввод
                if (wcin.fail())
                {
                    wcin.clear();                                                   // Очистка флага ошибки
                    wcin.ignore(numeric_limits<streamsize>::max(), '\n'); // Очистка буфера
                    continue;                                                            // Возврат в начало цикла
                }
                switch (choice)
                {
                case 1:
                    wcout << L"Введите 1 для включения или 0 для выключения: ";
                    wcin >> choice;
                    settings.changeRules(L"caseInsensitive", (choice == 1));
                    break;
                case 2:
                    wcout << L"Введите 1 для включения или 0 для выключения: ";
                    wcin >> choice;
                    settings.changeRules(L"ignoreNumbers", (choice == 1));
                    break;
                case 3:
                    wcout << L"Введите 1 для включения или 0 для выключения: ";
                    wcin >> choice;
                    settings.changeRules(L"ignoreSpecialChars", (choice == 1));
                    break;
                case 4:
                    wcout << L"Введите новый разрешенный алфавит: ";
                    getline(wcin, settings.allowedAlphabet);
                    break;
                case 5:
                    wcout << L"Введите 1 для включения или 0 для выключения: ";
                    wcin >> choice;
                    settings.changeRules(L"ignoreStopWords", (choice == 1));
                    break;
                case 6:
                {
                    wcout << L"Введите новое стоп-слово: ";

                    wstring stopWord;

                    // Сброс состояния потока и очистка буфера
                    if (wcin.fail())
                    {
                        wcin.clear();                                                    // Сброс флага ошибки
                        wcin.ignore(numeric_limits<streamsize>::max(), L'\n'); // Очистка буфера
                    }

                    getline(wcin, stopWord);

                    // Проверка на пустую строку
                    if (stopWord.empty())
                    {
                        wcout << L"Пусто (введите корректное стоп-слово).\n";
                        break;
                    }

                    settings.stopWords.insert(stopWord);
                    wcout << L"Стоп-слово добавлено: " << stopWord << "\n";
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
        catch (const exception &e)
        {
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

    void TextAnalyzer::saveResults(const wstring &filename) const
    {
        try
        {
            // Преобразование имени файла для разных ОС
#ifdef _WIN32
            // Для Windows: используем широкие символы напрямую
            ofstream file(filename);
#else
            // Для Unix: конвертируем wstring в UTF-8 string
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            string narrowName = converter.to_bytes(filename);
            ofstream file(narrowName);
#endif

            if (!file.is_open())
            {
                wcerr << L"ОШИБКА: Не удалось открыть файл для записи: " << filename << endl;
                return;
            }

            // Записываем результаты в файл
            file << "=== Результаты анализа ===\n";

            // Если результатов нет, выводим сообщение
            if (results.empty())
            {
                file << "Нет данных для сохранения.\n";
                file.close();
                wcout << L"Результаты отсутствуют. Файл сохранен без данных." << endl;
                return;
            }

            // Сохраняем результаты для каждого источника
            for (const auto &result : results)
            {
                file << "\n=== Результаты для источника: " << converter.to_bytes(result.sourceIdentifier) << " ===\n";

                // Топ 10 самых часто встречающихся слов
                file << "Топ 10 самых часто встречающихся слов:\n";
                printTopWordsToFile(10, result.sourceIdentifier, file);

                // Топ 5 самых длинных слов
                file << "\nТоп 5 самых длинных слов:\n";
                printLongestWordsToFile(5, result.sourceIdentifier, file);

                // Количество предложений
                file << "\nКоличество предложений: ";
                printSentenceCountToFile(result.sourceIdentifier, file);
            }

            file.close();
            wcout << L"Результаты успешно сохранены в файл: " << filename << endl;
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }

    void TextAnalyzer::printTopWordsToFile(size_t count, const wstring &sourceIdentifier, ofstream &file) const
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

            if (!targetResults)
            {
                wstring_convert<codecvt_utf8<wchar_t>> converter;
                file << "DEBUG: Результаты не найдены для " << converter.to_bytes(sourceIdentifier) << endl;
                return;
            }

            vector<pair<wstring, int>> words(targetResults->wordFrequency.begin(), targetResults->wordFrequency.end());
            sort(words.begin(), words.end(), [](const auto &a, const auto &b)
                      { return a.second > b.second; });

            for (size_t i = 0; i < min(count, words.size()); ++i)
            {
                wstring_convert<codecvt_utf8<wchar_t>> converter;
                file << "  " << converter.to_bytes(words[i].first) << ": " << words[i].second << " раз\n";
            }
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }

    void TextAnalyzer::printLongestWordsToFile(size_t count, const wstring &sourceIdentifier, ofstream &file) const
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

            if (!targetResults)
            {
                wstring_convert<codecvt_utf8<wchar_t>> converter;
                file << "DEBUG: Результаты не найдены для " << converter.to_bytes(sourceIdentifier) << endl;
                return;
            }

            size_t printed = 0;
            for (auto it = targetResults->wordsByLength.rbegin(); it != targetResults->wordsByLength.rend() && printed < count; ++it)
            {
                for (const auto &word : it->second)
                {
                    if (printed >= count)
                        break;
                    wstring_convert<codecvt_utf8<wchar_t>> converter;
                    file << "  " << converter.to_bytes(word) << " (длина: " << it->first << ")\n";
                    printed++;
                }
            }
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }

    void TextAnalyzer::printSentenceCountToFile(const wstring &sourceIdentifier, ofstream &file) const
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

            if (!targetResults)
            {
                wstring_convert<codecvt_utf8<wchar_t>> converter;
                file << "DEBUG: Результаты не найдены для " << converter.to_bytes(sourceIdentifier) << endl;
                return;
            }

            file << targetResults->sentenceCount << endl;
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }

    void TextAnalyzer::loadResults(const std::wstring& filename) {
        try {
            // Преобразование имени файла для разных ОС
#ifdef _WIN32
            std::wifstream file(filename);
            file.imbue(std::locale("Russian"));
#else
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::string narrowName = converter.to_bytes(filename);
            std::wifstream file(narrowName);
            file.imbue(std::locale("ru_RU.UTF-8"));
#endif

            if (!file.is_open()) {
                std::wcerr << L"ОШИБКА: Не удалось открыть файл для чтения: " << filename << std::endl;
                return;
            }

            // Очистка текущих результатов
            results.clear();

            std::wstring line;
            AnalysisResult currentResults;
            std::wstring currentSource;

            while (std::getline(file, line)) {
                std::wcout << L"Прочитанная строка: [" << line << L"]" << std::endl;
// Начало нового блока результатов
                if (line.find(L"=== Результаты для источника: ") != std::wstring::npos) {
                    if (!currentSource.empty()) {
                        results.push_back(currentResults);
                        currentResults = AnalysisResult(); // Сброс текущих результатов
                    }

                    // Извлечение имени источника
                    size_t start = line.find(L": ") + 2;
                    size_t end = line.find(L" ===");
                    currentSource = line.substr(start, end - start);
                    currentResults.sourceIdentifier = currentSource;
                } else if (line.find(L"Топ") != std::wstring::npos) {
                    // Обработка топовых слов
                    std::wregex regex(L"Топ (\\d+) самых часто встречающихся слов для ([-а-яА-ЯёЁa-zA-Z]+):");
                    std::wsmatch match;
                    if (std::regex_search(line, match, regex)) {
                        currentSource = match[2];
                        currentResults.sourceIdentifier = currentSource;
                    }
                } else if (line.find(L": ") != std::wstring::npos) {
                    // Обработка строк с частотами слов
                    size_t colonPos = line.find(L": ");
                    std::wstring word = line.substr(0, colonPos);
                    int count = std::stoi(line.substr(colonPos + 2));
                    currentResults.wordFrequency[word] = count;
                } else if (line.find(L"Количество предложений для") != std::wstring::npos) {
                    // Обработка количества предложений
                    std::wregex regex(L"Количество предложений для ([-а-яА-ЯёЁa-zA-Z]+): (\\d+)");
                    std::wsmatch match;
                    if (std::regex_search(line, match, regex)) {
                        currentResults.sentenceCount = std::stoi(match[2]);
                    }
                }
            }

            // Добавление последнего блока результатов
            if (!currentSource.empty()) {
                results.push_back(currentResults);
            }

            file.close();
            std::wcout << L"Результаты успешно загружены из файла: " << filename << std::endl;
        } catch (const std::exception& e) {
            std::wcerr << L"ОШИБКА: " << e.what() << std::endl;
        }
    }

    void TextAnalyzer::compareResults(const wstring &sourceIdentifier1, const wstring &sourceIdentifier2) const
    {
        try
        {
            const AnalysisResult *results1 = nullptr;
            const AnalysisResult *results2 = nullptr;

            // Поиск результатов для первого источника
            for (const auto &result : results)
            {
                if (result.sourceIdentifier == sourceIdentifier1)
                {
                    results1 = &result;
                    break;
                }
            }

            // Поиск результатов для второго источника
            for (const auto &result : results)
            {
                if (result.sourceIdentifier == sourceIdentifier2)
                {
                    results2 = &result;
                    break;
                }
            }

            if (!results1 || !results2)
            {
                wcerr << L"ОШИБКА: Не удалось найти результаты для одного из источников." << endl;
                return;
            }

            // Сравнение количества предложений
            wcout << L"\n=== Сравнение количества предложений ===\n";
            wcout << sourceIdentifier1 << L": " << results1->sentenceCount << L" предложений\n";
            wcout << sourceIdentifier2 << L": " << results2->sentenceCount << L" предложений\n";
            wcout << L"Разница: " << abs(results1->sentenceCount - results2->sentenceCount) << L" предложений\n";

            // Сравнение частот слов
            wcout << L"\n=== Сравнение частот слов ===\n";
            map<wstring, int> commonWords;
            for (const auto &[word, count] : results1->wordFrequency)
            {
                if (results2->wordFrequency.find(word) != results2->wordFrequency.end())
                {
                    commonWords[word] = abs(count - results2->wordFrequency.at(word));
                }
            }

            if (!commonWords.empty())
            {
                wcout << L"Слова с наибольшей разницей в частоте:\n";
                vector<pair<wstring, int>> sortedCommonWords(commonWords.begin(), commonWords.end());
                sort(sortedCommonWords.begin(), sortedCommonWords.end(),
                          [](const auto &a, const auto &b)
                          { return a.second > b.second; });

                for (size_t i = 0; i < min(sortedCommonWords.size(), size_t(5)); ++i)
                {
                    wcout << L"  " << sortedCommonWords[i].first << L": разница " << sortedCommonWords[i].second << L"\n";
                }
            }
            else
            {
                wcout << L"Нет общих слов для сравнения.\n";
            }

            // Сравнение самых длинных слов
            wcout << L"\n=== Сравнение самых длинных слов ===\n";
            auto longestWords1 = results1->wordsByLength.rbegin()->second;
            auto longestWords2 = results2->wordsByLength.rbegin()->second;

            wcout << sourceIdentifier1 << L": ";
            for (const auto &word : longestWords1)
            {
                wcout << word << L" ";
            }
            wcout << L"\n";

            wcout << sourceIdentifier2 << L": ";
            for (const auto &word : longestWords2)
            {
                wcout << word << L" ";
            }
            wcout << L"\n";
        }
        catch (const exception &e)
        {
            wcerr << L"ОШИБКА: " << e.what() << endl;
        }
    }
}