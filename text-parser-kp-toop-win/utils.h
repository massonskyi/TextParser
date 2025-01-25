#ifndef UTILS_H
#define UTILS_H
#include <string>

namespace std {
    wstring convertToWstring(const string& str);
    void showProgressBar(size_t current, size_t total, const std::wstring& message = L"Анализирую...");
    bool isValidString(std::wstring& command);
}
#endif /* UTILS_H */