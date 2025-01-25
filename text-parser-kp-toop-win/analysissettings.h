#ifndef ANALYSISSETTINGS_H
#define ANALYSISSETTINGS_H

#include <unordered_map>
#include <string>
#include <set>

namespace std {

    class AnalysisSettings final {
    public:
        using stack_rules = unordered_map<wstring, bool>;

        wstring allowedAlphabet;
        set<wstring> stopWords; // Список стоп-слов
        wstring workingDirectory; // Директория для поиска файлов

        AnalysisSettings();
        AnalysisSettings(const AnalysisSettings& other);
        AnalysisSettings(AnalysisSettings&& other) noexcept ;

        AnalysisSettings& operator=(const AnalysisSettings& other);
        AnalysisSettings& operator=(AnalysisSettings&& other) noexcept ;


        void changeRules(const wstring& rulesName, bool rulesValue);
        bool getRules(const wstring& rulesName) const;

		void addToBackAlphabet(const wstring& stopWord);
		void setNewAlphabet(const wstring& newAlphabet);

        // Новые методы для работы с директорией
        void setWorkingDirectory(const wstring& directory);
        wstring getWorkingDirectory() const;

    private:
        stack_rules rules;
    };
    
}
#endif /* ANALYSISSETTINGS_H */ 