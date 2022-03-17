#include "Common/Error.h"
#include "Common/utils.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace CXX {

	std::string string_with_arrows(const std::string& content, const Position& start, const Position& end)
	{
		std::string result;

		// indices
		size_t idx_start = content.rfind('\n', start.index);
		idx_start = (idx_start == std::string::npos) ? 0 : idx_start;
		size_t idx_end = content.find('\n', idx_start + 1);

		// Generate Line
		int line_count = end.row - start.row + 1;
		for (int i = 0; i < line_count; i++)
		{
			std::string line = content.substr(idx_start, idx_end - idx_start);

			// calculate line column
			int col_start = i == 0 ? start.column : 0;
			int col_end = (i == line_count - 1) ? end.column : line.length() - 1;

			// append to result
			result += line + "\n";

			for (int j = 0; j < col_start; j++)
				result += " ";

			for (int j = 0; j < (col_end - col_start); j++)
				result += "^";

			// Recalculate indices
			idx_start = idx_end;
			idx_end = content.find('\n', idx_start + 1);
			idx_end = (idx_end == std::string::npos) ? content.length() : idx_end;
		}

		for (auto it = result.begin(); it != result.end();)
		{
			if ((*it) == '\t')
				it = result.erase(it);
			else
				it++;
		}

		return result;
	}

	Error::Error(const Position& start, const Position& end, std::string name, std::string details) :pos_start(start), pos_end(end)
	{
		generate_message(name, details);
	}

	const char* Error::what() const noexcept
	{
		return message.c_str();
	}

	void Error::generate_message(const std::string& error_name, const std::string& details)
	{
		message = format("%s: %s\n", error_name.c_str(), details.c_str());
		message += format("File %s, line %s\n\n", this->pos_start.fileName.data(),
			std::to_string(this->pos_start.row + 1).c_str());
		message += string_with_arrows(this->pos_start.fileContent.data(), this->pos_start, this->pos_end);
	}

	void ErrorReporter::report(const std::exception& error)
	{
		std::cerr << error.what() << "\n";
		errorCount++;
	}

	void ErrorReporter::reset()
	{
		errorCount = 0;
	}

	int ErrorReporter::count()
	{
		int count = errorCount;
		errorCount = 0;
		return count;
	}

	int ErrorReporter::errorCount = 0;

}