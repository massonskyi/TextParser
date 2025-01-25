#ifndef TEXTANALYZER_H
#define TEXTANALYZER_H

#include "analyzer.h"
#include "analysissettings.h"

#include <map>

namespace std {
        
    class TextAnalyzer : public Analyzer{
    public:
        struct AnalysisResult {
            map<wstring, int> wordFrequency;
            map<size_t, set<wstring>> wordsByLength;
            int sentenceCount = 0;
            wstring sourceIdentifier;
        };
        using result_vector = vector<AnalysisResult>;

        TextAnalyzer() = default;

        explicit TextAnalyzer(const AnalysisSettings& settings = AnalysisSettings());

        vector<wstring> readFile(const wstring& filename);
        vector<wstring> split(const wstring& str, wchar_t delimiter = L' ');
        void analyzeFiles(const vector<wstring>& filePaths, const wstring& sourceIdentifier) override;
        void printTopWords(size_t count, const wstring& sourceIdentifier = L"") const override;
        void printLongestWords(size_t count, const wstring& sourceIdentifier = L"") const override;
        void printSentenceCount(const wstring& sourceIdentifier = L"") const override;
        void printCurrentResults() const;
        void printSettings() const override;
        void changeSettings() override;
        void clearResults() override;
        void saveResults(const wstring& filename) const override;
        void printTopWordsToFile(size_t count, const std::wstring& sourceIdentifier, std::wofstream& file) const ;
        void printLongestWordsToFile(size_t count, const std::wstring& sourceIdentifier, std::wofstream& file) const ;
        void printSentenceCountToFile(const std::wstring& sourceIdentifier, std::wofstream& file) const ;
        void loadResults(const wstring& filename) override;
        result_vector loadExternalResult(const wstring& filename);
        void compareResults(const wstring& sourceIdentifier1, const wstring& sourceIdentifier2) override;

        void processFile(const wstring& filePath ,const std::wstring &sourceIdentifier);
        void analyzeDirectory(const wstring& directory);
        void parseLongestWords(const wstring& line, map<size_t, set<wstring>>& wordsByLength);
        void parseWordFrequency(const wstring& line, map<wstring, int>& wordFrequency);
        TextAnalyzer::AnalysisResult parseAnalysisResult(const wstring& filename);
        [[nodiscard]]
        wstring getCurrentDirectory()const{
            return settings.getWorkingDirectory();
        }

        void setCurrentDirectory(const wstring dir){
            settings.setWorkingDirectory(dir);
        }
        void printMostlyAphabet();
        ~TextAnalyzer() = default;
    private:



        AnalysisSettings settings;
        result_vector results;


        bool isValidWord(const wstring& word) const;
        bool isStopWord(const wstring& word) const;
        wstring processWord(wstring word) const;
    };
};
#endif /* TEXTANALYZER_H */