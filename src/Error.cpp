#include "Error.h"

namespace Chronos
{
	std::string error_to_string(ErrorType e)
	{
		switch (e)
		{
		case ErrorType::ILLEGAL_CHAR: return "ILLEGAL_CHAR";
		case ErrorType::EXPECTED_CHAR: return "EXPECTED_CHAR";
		case ErrorType::INVALID_SYNTAX: return "INVALID_SYNTAX";
		case ErrorType::RUNTIME: return "RUNTIME";
		case ErrorType::UNDEFINED_OPERATOR: return "UNDEFINED_OPERATOR";
		}

		ASSERT(false, "to_string not defined for this error type");
		exit(-1);
	}

	std::string Error::generate_message(std::vector<File>& files)
	{
		std::string message = "";
		std::string file_name = files[start_pos.file_nr].name;
		message += error_to_string(type) + ": " + details + "\n";
		message += get_error_preview(start_pos.file_nr, files);

		return message;
	}

	std::string Error::get_error_preview(std::size_t file_nr, std::vector<File>& files)
	{
		std::string res = "";
		auto& file = files[file_nr];
		auto& text = file.text;

		int start = text.substr(0, start_pos.index).rfind("\n");
		if (start == std::string::npos) start = 0;
		int end = text.substr(start + 1).find("\n");
		if (end == std::string::npos) end= text.size() - start + 1;
		end += start + 1;

		int count = end_pos.line - start_pos.line + 1;

		for (int i = 0; i < count; i++)
		{
			 std::string line = text.substr(start, end);
			 int col_start = 0;
			 if (i == 0) col_start = start_pos.column;
			 int col_end = line.size() - 1;
			 if (i == count - 1) col_end = end_pos.column;

			 res += line + "\n";
			 res += std::string(col_start, ' ');
			 res += std::string(col_end - col_start, '~');

			 start = end;
			 end = text.substr(0, start).find('\n');
			 if (end == std::string::npos) end = text.size();
		}
		return res;
	}
}
