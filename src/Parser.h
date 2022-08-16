#pragma once

#include <vector>
#include <functional>

#include "Debug.h"
#include "Lexer.h"
#include "Error.h"

namespace Chronos
{
	struct Node;

	void delete_nodes(Node* nodes);

	enum class ValueType
	{
		INT,
		FLOAT,
		POINTER,

		NONE,
	};


	enum class NodeType
	{
		NUM = 0,
		BINOP,
		UNRYOP,
		ASSIGN,
		ACCESS,

		ROOT,
	};

	namespace NodeValues
	{
		struct Root
		{
			std::vector<Node*> nodes;
		};

		struct UnryOp
		{
			TokenType type;
			Node* right;
		};

		struct AssignOp
		{
			std::string var;
			Node* expr;
		};

		struct BinOp
		{
			Node* left;
			TokenType type;
			Node* right;
		};
	}

	enum class ParseRes : uint8_t
	{
		OK = 1,
		ERROR = 0
	};

	using ParseResult = std::variant<Error, Node*>;
	using NodeValue = std::variant<int, std::string, Token, NodeValues::UnryOp, NodeValues::AssignOp, NodeValues::BinOp, Node*, NodeValues::Root>;

	struct Node
	{
		NodeType type;
		NodeValue value;

		Position start_pos;
		Position end_pos;

		ValueType value_type = ValueType::NONE;
	};

	//using ParseResult = Result<Node*, Error>;


	std::string to_string(const Node& n);

	class Parser
	{
	private:
		std::deque<Token> m_Tokens = {};
		size_t m_TokenIndex = 0;
		Token* m_CurrentToken = nullptr;

		void advance();
		void retreat();

		ParseResult atom();
		ParseResult factor();
		ParseResult wrap_callable(Node* node);
		ParseResult callable();
		ParseResult term();
		ParseResult arith_expression();
		ParseResult comp_expression();
		ParseResult binop_expression(std::function<ParseResult(Parser*)> func_a, std::vector<TokenType> ops,
			std::function<ParseResult(Parser*)> func_b);
		ParseResult expression();

	public:
		void load_tokens(std::deque<Token> tokens)
		{
			m_Tokens = std::move(tokens);
			m_TokenIndex = 0;
			if (!m_Tokens.empty()) 
			{
				m_CurrentToken = &m_Tokens[0];
			}
		}

		ParseResult parse_nodes();

	};



}

