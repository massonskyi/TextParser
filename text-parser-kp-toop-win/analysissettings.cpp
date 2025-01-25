#include "analysissettings.h"

#include <iostream>

namespace std {

    AnalysisSettings::AnalysisSettings() {
        rules[L"caseInsensitive"] = false;
        rules[L"ignoreNumbers"] = false;
        rules[L"ignoreSpecialChars"] = false;
        rules[L"ignoreStopWords"] = false;
        workingDirectory = L"./";
    }

    AnalysisSettings::AnalysisSettings(const AnalysisSettings& other) {
        rules[L"caseInsensitive"] = other.getRules(L"caseInsensitive");
        rules[L"ignoreNumbers"] = other.getRules(L"ignoreNumbers");
        rules[L"ignoreSpecialChars"] = other.getRules(L"ignoreSpecialChars");
        rules[L"ignoreStopWords"] = other.getRules(L"ignoreStopWords");
        allowedAlphabet = other.allowedAlphabet;
        stopWords = other.stopWords;
        workingDirectory = other.workingDirectory;
    }

    AnalysisSettings::AnalysisSettings(AnalysisSettings&& other) noexcept {
        rules[L"caseInsensitive"] = move(other.getRules(L"caseInsensitive"));
        rules[L"ignoreNumbers"] = move(other.getRules(L"ignoreNumbers"));
        rules[L"ignoreSpecialChars"] = move(other.getRules(L"ignoreSpecialChars"));
        rules[L"ignoreStopWords"] = move(other.getRules(L"ignoreStopWords"));
        allowedAlphabet = move(other.allowedAlphabet);
        stopWords = move(other.stopWords);
        workingDirectory = move(other.workingDirectory);
    }

    AnalysisSettings& AnalysisSettings::operator=(const AnalysisSettings& other) {
        rules[L"caseInsensitive"] = other.getRules(L"caseInsensitive");
        rules[L"ignoreNumbers"] = other.getRules(L"ignoreNumbers");
        rules[L"ignoreSpecialChars"] = other.getRules(L"ignoreSpecialChars");
        rules[L"ignoreStopWords"] = other.getRules(L"ignoreStopWords");
        allowedAlphabet = other.allowedAlphabet;
        stopWords = other.stopWords;
        workingDirectory = other.workingDirectory;
        return *this;
    }

    AnalysisSettings& AnalysisSettings::operator=(AnalysisSettings&& other) noexcept {
        rules[L"caseInsensitive"] = move(other.getRules(L"caseInsensitive"));
        rules[L"ignoreNumbers"] = move(other.getRules(L"ignoreNumbers"));
        rules[L"ignoreSpecialChars"] = move(other.getRules(L"ignoreSpecialChars"));
        rules[L"ignoreStopWords"] = move(other.getRules(L"ignoreStopWords"));
        allowedAlphabet = move(other.allowedAlphabet);
        stopWords = move(other.stopWords);
        workingDirectory = move(other.workingDirectory);
        return *this;
    }

    void AnalysisSettings::changeRules(const wstring& rulesName, bool rulesValue) {
        rules[rulesName] = rulesValue;
    }

    bool AnalysisSettings::getRules(const wstring& rulesName) const {
        auto it = rules.find(rulesName);
        if (it != rules.end()) {
            return it->second;
        }
        wcerr << L"Правила не существует" << endl;
        return false;
    }
    void
    AnalysisSettings::setWorkingDirectory(const wstring& directory) {
        workingDirectory = directory;
    }
    void
    AnalysisSettings::addToBackAlphabet(const wstring& new_word) {
	    allowedAlphabet += new_word;
    }

    void
    AnalysisSettings::setNewAlphabet(const wstring& newAlphabet) {
		allowedAlphabet = newAlphabet;
    }
    wstring
        AnalysisSettings::getWorkingDirectory() const {
        return workingDirectory;
    }
}