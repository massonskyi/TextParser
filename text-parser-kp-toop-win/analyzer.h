#ifndef ANALYZER_H
#define ANALYZER_H

#include <vector>
#include <string>

namespace std{

    class Analyzer {
    public:
        virtual void analyzeFiles(const vector<wstring>& filePaths, const wstring& sourceIdentifier) = 0;
        virtual void printTopWords(size_t count, const wstring& sourceIdentifier = L"") const = 0;
        virtual void printLongestWords(size_t count, const wstring& sourceIdentifier = L"") const = 0;
        virtual void printSentenceCount(const wstring& sourceIdentifier = L"") const = 0;
        virtual void printSettings() const = 0;
        virtual void changeSettings() = 0;
        virtual void clearResults() = 0;
        virtual void saveResults(const wstring& filename) const = 0;
        virtual void loadResults(const wstring& filename) = 0;
        virtual void compareResults(const wstring& sourceIdentifier1, const wstring& sourceIdentifier2) const = 0;
    };

}
#endif /* ANALYZER_H */