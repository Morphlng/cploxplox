#include "Common/Position.h"
#include <algorithm>
#include <utility>

namespace CXX {

    Position::Position(int idx, int line, int col, std::string filename, std::string ftx) : index(idx), row(line),
        column(col),
        fileName(std::move(filename)),
        fileContent(std::move(ftx))
    {
    }

    Position::Position(const Position& other)
    {
        this->column = other.column;
        this->row = other.row;
        this->index = other.index;
        this->fileName = other.fileName;
        this->fileContent = other.fileContent;
    }

    void Position::advance(char current_char)
    {
        this->index++;
        this->column++;

        if (current_char == '\n')
        {
            this->row++;
            this->column = 0;
        }
    }

    const Position Position::preset = Position();

}