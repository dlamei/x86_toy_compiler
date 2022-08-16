#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <variant>
#include <optional>

#include "Debug.h"
#include "Error.h"

namespace Chronos
{

	enum class TokenType
	{
		#define TOKEN_TYPE(a) a,
		#include "Token.h"
		/*
		INT = 0,
		FLOAT,
		ADD,
		SUB,
		MUL,
		DIV,
		NOT,
		LROUND,
		RROUND,
		LCURLY,
		RCURLY,
		ASSIGN,
		SEMICLN,
		ID,
		IF,
		ELSE,

		KW_AND,
		KW_OR,
		EQUAL,
		LESS,
		GREATER,
		LESS_EQ,
		GREATER_EQ,

		NONE,
		*/
	};

	using TokenValue = std::variant<int, float, std::string>;

	struct Token
	{
		TokenType type;
		TokenValue value;

		Position start_pos;
		Position end_pos;

		Token(TokenType t, TokenValue v, Position a, Position b)
			: type(t), value(v), start_pos(a), end_pos(b) {}

		Token(TokenType t, TokenValue v, Position pos)
			: type(t), value(v), start_pos(pos), end_pos(pos) 
		{
			end_pos.column++;
			end_pos.index++;
		}
	};

	std::string to_string(const TokenType t);
	std::string to_string(const Token& t);
	std::string to_string(const Position& pos);

	enum class LexerRes : uint8_t
	{
		OK = 1,
		ERROR = 0
	};

	using LexerResult = std::variant<Error, Token>;

	class Lexer
	{
		private:
			std::deque<Token> m_Tokens;
			Error m_Error;
			bool m_HasError = false;

			size_t m_Line = 0;
			size_t m_Column = 0;
			size_t m_Index = 0;

			std::size_t m_TextSize = 0;
			const char *m_Text = nullptr;
			const char *m_CharPtr = nullptr;


			void advance();
			Position get_current_pos();

			void set_error(Error e);

			std::optional<Error> expect_char(const char c);
			Token make_number();
			Token make_identifier();
			Token make_equal();
			Token make_less();
			Token make_greater();
			LexerResult make_and();
			LexerResult make_or();

		public:

			Lexer() {}

			std::deque<Token> get_tokens()
			{
				return m_Tokens;
			}

			void load_text(const char* text, size_t size);

			void parse_tokens();
			void print_tokens();

			void clear();

			Token peek();
			void pop();
			bool has_error() { return m_HasError; }
			Error get_error() { return m_Error; }

	};
}
