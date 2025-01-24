#ifndef CORE_H
#define CORE_H

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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <clocale>
#endif

#include "filemanager.h"
#include "analysissettings.h"
#include "textaanalyzer.h"

#endif /* CORE_H */