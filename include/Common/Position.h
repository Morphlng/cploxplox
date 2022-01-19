#pragma once

#include <string>

namespace CXX {

    // Track line number, column number and current index
    // also the FileName, and FileContent
    class Position
    {

    public:
        Position(int idx = -1, int line = 0, int col = -1, std::string filename = "", std::string ftx = "");

        Position(const Position& other);

        void advance(char current_char = '\0');

        static const Position preset;

    public:
        int index;
        int row;
        int column;
        std::string fileName;
        std::string fileContent;
    };

}