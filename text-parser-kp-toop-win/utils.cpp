#include "utils.h"

#include <codecvt>
#include <locale>
#include <string>
#include <iostream>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <clocale>
#endif
#include <iomanip>

namespace std {

    wstring convertToWstring(const string& str) {
    #ifdef _WIN32
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
        wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
        return wstr;
    #else
        wstring_convert<codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(str);
    #endif
    }
    // Функция для отображения прогресс-бара
    void showProgressBar(size_t current, size_t total, const std::wstring& message) {
    const size_t barWidth = 50; // Ширина прогресс-бара
    float progress = static_cast<float>(current) / total;
    size_t pos = static_cast<size_t>(barWidth * progress);

    std::wcout << L"\r" << message << L" ["; // Возврат каретки в начало строки
    for (size_t i = 0; i < barWidth; ++i) {
        if (i < pos) std::wcout << L"=";
        else if (i == pos) std::wcout << L">";
        else std::wcout << L" ";
    }
    std::wcout << L"] " << std::fixed << std::setprecision(2) << (progress * 100.0) << L"%";
    std::wcout.flush(); // Очистка буфера вывода
}

    bool isValidString(std::wstring& command) {
        if (command.empty()) {
            return false;
        }
        try {
            // Попытка преобразовать строку в число
            auto x = std::stoi(command);
            return true; // Если преобразование успешно, выходим из цикла
        }
        catch (const std::invalid_argument&) {
            return false;
        }
        catch (const std::out_of_range&) {
            return false;
        }
    }
}