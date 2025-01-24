#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string> 

namespace std {

    class FileManager final{
    public:
        static void createTestFile(const wstring& filename, const wstring& content);
    };

}
#endif /* FILEMANAGER_H */