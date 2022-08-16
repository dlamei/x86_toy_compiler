#pragma once

#include <string>
#include <iostream>

#include "Debug.h"
#include <vector>

namespace Chronos
{
	struct Position
	{
		std::size_t index = 0;
		std::size_t line = 0;
		std::size_t column = 0;

		std::size_t file_nr = 0;
		std::size_t offset = 0;
	};

	struct File
	{
		std::string name;
		std::string text;
	};

	class FileManager
	{
	private:
		std::vector<File> m_Files = {};
		size_t m_CurrentLine = -1;

	public:

		void clear()
		{
			m_Files.clear();
			m_CurrentLine = -1;
		}

		void add_file(std::string name, std::string text)
		{
			File file = { name, text };
			m_Files.push_back(file);
			m_CurrentLine = 0;
		}

		std::vector<File>& get_files()
		{
			return m_Files;
		}

		void add_line(std::string line, std::string file_name)
		{
			if (m_Files.empty() || m_Files.front().name != file_name)
			{
				add_file(file_name, line);
			}
			else
			{
				File& last = m_Files.front();
				last.text += line;
				++m_CurrentLine;
			}
		}
	};

	enum class ErrorType
	{
		ILLEGAL_CHAR = 0,
		EXPECTED_CHAR,
		INVALID_SYNTAX,
		RUNTIME,
		UNDEFINED_OPERATOR,

		NONE
	};

	std::string error_to_string(ErrorType e);

	struct Error
	{
		ErrorType type = ErrorType::NONE;
		std::string details;

		Position start_pos;
		Position end_pos;

		std::string generate_message(std::vector<File>& files);
		std::string get_error_preview(std::size_t file_nr, std::vector<File>& files);
	};


}
