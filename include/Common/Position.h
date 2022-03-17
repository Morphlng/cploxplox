#pragma once

#include <string_view>
#include <string>

namespace CXX {

    // Track line number, column number and current index
    // also the FileName, and FileContent
    class Position
    {

    public:
        Position(int idx = -1, int line = 0, int col = -1, const std::string& filename = "", const std::string& ftx = "");

        Position(const Position& other);

        void advance(char current_char = '\0');

        static const Position preset;

    public:
        int index;
        int row;
        int column;
#ifdef USE_STRING_VIEW
        std::string_view fileName;
        std::string_view fileContent;
#else
        std::string fileName;
        std::string fileContent;
#endif
    };

}