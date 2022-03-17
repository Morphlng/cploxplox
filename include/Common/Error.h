#pragma once

#include <stdexcept>
#include "Common/Position.h"

namespace CXX {

	class Error :public std::exception
	{
	public:
		Error(const Position& pos_start, const Position& pos_end, std::string error_name,
			std::string details);

		const char* what() const noexcept override;

	public:
		friend class ErrorReporter;
		Position pos_start;
		Position pos_end;

	private:
		std::string message;

		void generate_message(const std::string& error_name, const std::string& details);
	};

	class ErrorReporter
	{
	public:
		static void report(const std::exception& error);
		static void reset();
		static int count();

	public:
		static int errorCount;
	};

}