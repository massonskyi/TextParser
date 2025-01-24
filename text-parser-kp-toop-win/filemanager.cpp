#include "filemanager.h"

#include <codecvt>
#include <locale>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <clocale>
#endif

namespace std{
    void FileManager::createTestFile(const wstring& filename, const wstring& content) {
    #ifdef _WIN32
        ofstream file(filename, ios::binary);
    #else
        string narrowName = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(filename);
        ofstream file(narrowName, ios::binary);
    #endif

        if (file.is_open()) {
    #ifdef _WIN32
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), (int)content.size(), NULL, 0, NULL, NULL);
            string strTo(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, content.c_str(), (int)content.size(), &strTo[0], size_needed, NULL, NULL);
    #else
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            string strTo = converter.to_bytes(content);
    #endif

            file << strTo;
            file.close();
            wcout << L"DEBUG: Файл создан: " << filename << endl;
        } else {
            wcerr << L"ОШИБКА: Не удалось создать файл: " << filename << endl;
        }
    }
}