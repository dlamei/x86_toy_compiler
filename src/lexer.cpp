#include "Lexer.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <unordered_map>

namespace Chronos
{
	const std::string LETTERS = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	bool is_space(char c)
	{
		switch (c)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			return true;
		default:
			return false;
		}
	}

	bool is_letter(char c)
	{
		int pos = LETTERS.find(c);
		return (pos != std::string::npos);
	}

	bool is_digit(char c)
	{
		switch (c)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return true;
		default: return false;
		}
	}

	std::string to_string(const Position& pos)
	{
		std::string s = "indx: ";
		s += std::to_string(pos.index) + ", line: " + std::to_string(pos.line) + ", column: " + std::to_string(pos.column);
		return s;
	}

	TokenType get_tokentype(const char* s)
	{
		if (s == "&&") return TokenType::KW_AND;
		else if (s == "||") return TokenType::KW_OR;
		else return TokenType::NONE;
	}

	std::string to_string(const TokenType t)
	{
		switch (t)
		{
			#define TOKEN_TYPE(a) case TokenType::a: return #a;
			#include "Token.h"
		default:
			ASSERT(false, "to_string not defined for tokentype");
			exit(-1);
		}
	}

	std::string to_string(const Token& t)
	{
		std::string s;

		switch (t.type)
		{
		case TokenType::INT:
			s += "INT(";
			s += std::to_string(std::get<int>(t.value));
			s += ")";
			break;
		case TokenType::FLOAT:
			s += "FLOAT(";
			s += std::to_string(std::get<float>(t.value));
			s += ")";
			break;
		case TokenType::ID:
			s += "ID(";
			s += std::get<std::string>(t.value);
			s += ")";
			break;


		default:
			return to_string(t.type);
		}

#ifdef PRINT_POS
		s += " start: " + to_string(t.start_pos);
		s += ", end: " + to_string(t.end_pos);
#endif

		return s;
	}

	void Lexer::load_text(const char* text, size_t size)
	{
		m_TextSize = size;
		m_Text = text;
		m_CharPtr = text;
	}


	void Lexer::advance()
	{
		if (m_CharPtr == nullptr) return;
		switch (*m_CharPtr)
		{
		case '\n':
			++m_Line;
			m_Column = 0;
			break;

		default:
			++m_Column;
			break;
		}


		if (m_Index >= m_TextSize)
		{
			m_CharPtr = nullptr;
		}
		else
		{
			++m_CharPtr;
			++m_Index;
		}
	}

	Position Lexer::get_current_pos()
	{
		return Position({ m_Index, m_Line, m_Column });
	}

	void Lexer::set_error(Error e)
	{
		m_HasError = true;
		m_Error = e;
	}

	Token Lexer::peek()
	{
		return m_Tokens.back();
	}

	void Lexer::pop()
	{
		m_Tokens.pop_back();
	}

	void Lexer::parse_tokens()
	{
		while (m_CharPtr)
		{
			char c = *m_CharPtr;
			if (c == NULL) return;

			if (is_space(c))
			{
				advance();
				continue;
			}
			else if (is_digit(c))
			{
				m_Tokens.push_back(make_number());
				continue;
			}
			else if (is_letter(c))
			{
				m_Tokens.push_back(make_identifier());
				continue;
			}

			Position pos = get_current_pos();

			switch (c)
			{
			case '+':
				m_Tokens.push_back(Token(TokenType::ADD, { 0 }, pos));
				break;
			case '&':
			{
				auto token = make_and();
				if (token.index() == (int)LexerRes::ERROR) set_error(std::get<Error>(token));
				else m_Tokens.push_back(std::get<Token>(token));
				break;
			}
			case '|':
			{
				auto token = make_or();
				if (token.index() == (int)LexerRes::ERROR) set_error(std::get<Error>(token));
				else m_Tokens.push_back(std::get<Token>(token));
				break;
			}
			case '-':
				m_Tokens.push_back(Token(TokenType::SUB, { 0 }, pos));
				break;
			case '!':
				m_Tokens.push_back(Token(TokenType::NOT, { 0 }, pos));
				break;
			case '*':
				m_Tokens.push_back(Token(TokenType::MUL, { 0 }, pos));
				break;
			case '/':
				m_Tokens.push_back(Token(TokenType::DIV, { 0 }, pos));
				break;
			case '(':
				m_Tokens.push_back(Token(TokenType::LROUND, { 0 }, pos));
				break;
			case ')':
				m_Tokens.push_back(Token(TokenType::RROUND, { 0 }, pos));
				break;
			case '=':
				m_Tokens.push_back(make_equal());
				break;
			case '<':
				m_Tokens.push_back(make_less());
				break;
			case '>':
				m_Tokens.push_back(make_greater());
				break;

			default:
				m_HasError = true;
				advance();
				Position current_pos = get_current_pos();
				std::string details = "found char: ";
				details.push_back(c);
				m_Error = { ErrorType::ILLEGAL_CHAR, details, pos, current_pos};
				return;
			}

			advance();
		}
	}

	std::optional<Error> Lexer::expect_char(const char c)
	{
		if (*m_CharPtr != c)
		{
			Position start = get_current_pos();

			std::string details = "expected '";
			details.push_back(c);
			details += "', found: ";
			details.push_back(*m_CharPtr);
			advance();

			Position end = get_current_pos();
			return Error{ ErrorType::ILLEGAL_CHAR, details, start, end };
		}

		return {};
	}

	Token Lexer::make_number()
	{
		std::string num;
		Position start = get_current_pos();
		bool has_dot = false;

		while (m_CharPtr && (is_digit(*m_CharPtr) || *m_CharPtr == '.'))
		{
			if (*m_CharPtr == '.' && !has_dot)
			{
				has_dot = true;
			}
			else if (*m_CharPtr == '.' && has_dot)
			{
				break;
			}

			num += *m_CharPtr;
			advance();
		}

		Position end = get_current_pos();

		if (has_dot)
		{
			TokenValue value;
			value = std::stof(num);
			return Token(TokenType::FLOAT, value, start, end);
		}
		else
		{
			TokenValue value;
			value = std::stoi(num);
			return Token(TokenType::INT, value, start, end);
		}
	}

	Token Lexer::make_identifier()
	{
		std::string id = "";
		Position start = get_current_pos();

		std::string allowed = LETTERS;

		while (m_CharPtr && (allowed.find(*m_CharPtr) != std::string::npos))
		{
			id.push_back(*m_CharPtr);
			advance();
		}

		TokenType type = get_tokentype(id.c_str());
		Position end = get_current_pos();

		if (type != TokenType::NONE)
		{
			return Token(type, { 0 }, start, end);
		}
		else
		{
			return Token(TokenType::ID, id, start, end);
		}
	}

	Token Lexer::make_equal()
	{
		Position start = get_current_pos();
		TokenType type = TokenType::ASSIGN;
		advance();

		if (m_CharPtr && *m_CharPtr == '=')
		{
			advance();
			type = TokenType::EQUAL;
		}

		Position end = get_current_pos();

		return { type, 0, start, end };
	}

	Token Lexer::make_less()
	{
		Position start = get_current_pos();
		TokenType type = TokenType::LESS;
		advance();

		if (m_CharPtr && *m_CharPtr == '=')
		{
			advance();
			type = TokenType::LESS_EQ;
		}

		Position end = get_current_pos();

		return { type, 0, start, end };
	}

	Token Lexer::make_greater()
	{
		Position start = get_current_pos();
		TokenType type = TokenType::GREATER;
		advance();

		if (m_CharPtr && *m_CharPtr == '=')
		{
			advance();
			type = TokenType::GREATER_EQ;
		}

		Position end = get_current_pos();

		return { type, 0, start, end };
	}

	LexerResult Lexer::make_and()
	{
		Position start = get_current_pos();

		std::optional<Error> e;
		if (e = expect_char('&')) return e.value();
		advance();
		if (e = expect_char('&')) return e.value();

		Position end = get_current_pos();

		return Token(TokenType::KW_AND, 0, start, end);
	}

	LexerResult Lexer::make_or()
	{
		Position start = get_current_pos();

		std::optional<Error> e;
		if (e = expect_char('|')) return e.value();
		advance();
		if (e = expect_char('|')) return e.value();

		Position end = get_current_pos();

		return Token(TokenType::KW_OR, 0, start, end);
	}

	void Lexer::print_tokens()
	{
		std::cout << "size: " << m_Tokens.size() << "\n";
		for (Token& t : m_Tokens)
		{
			std::cout << to_string(t) << ", ";
		}
		std::cout << "\n";
	}
	void Lexer::clear()
	{
		m_HasError = false;

		m_Tokens.clear();
		m_Line = 0;
		m_Column = 0;
		m_Index = 0;

		m_TextSize = 0;
		m_Text = nullptr;
		m_CharPtr = nullptr;
	}
}
