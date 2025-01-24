cmake_minimum_required(VERSION 3.25)
project(TOSHA_KP)

set(CMAKE_CXX_STANDARD 23)

add_executable(TOSHA_KP
        main.cpp
        include/core.h
        include/analysissettings.h
        include/analyzer.h
        include/filemanager.h
        include/utils.h
        src/analysissettings.cpp
        src/filemanager.cpp
        src/textanalyzer.cpp
        src/utils.cpp
        lib/tabulate.hpp
)
