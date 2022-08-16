#include <iostream>
#include <algorithm>
#include <stack>

#include "Parser.h"

namespace Chronos
{
	using namespace NodeValues;

	void print_tokens(const std::deque<Token>& tokens)
	{
		std::cout << "\n";
		for (const Token& t : tokens)
		{
			std::cout << to_string(t) << ", ";
		}

		std::cout << "\n";
	}

	void delete_nodes(Node* root)
	{
		if (!root) return; 

		std::stack<Node*> nodes;
		nodes.push(root);

		while (!nodes.empty())
		{
			Node* n = nodes.top();
			nodes.pop();

			switch (n->type)
			{
			case NodeType::ROOT:
				for (Node* node : std::get<Root>(n->value).nodes) nodes.push(node);
				break;

			case NodeType::NUM:
				break;

			case NodeType::UNRYOP:
				nodes.push(std::get<UnryOp>(n->value).right);
				break;

			case NodeType::BINOP:
				nodes.push(std::get<BinOp>(n->value).right);
				nodes.push(std::get<BinOp>(n->value).left);
				break;

			case NodeType::ASSIGN:
				nodes.push(std::get<AssignOp>(n->value).expr);
				break;

			case NodeType::ACCESS:
				break;

			default:
				ASSERT(false, "delete for this type not defined");
				exit(-1);
			}

			delete n;
		}
	}

	std::string to_string(const Node& n)
	{
		std::string s;

		switch (n.type)
		{
		case NodeType::NUM:
			s += "NUM(";
			s += to_string(std::get<Token>(n.value));
			s += ")";
			break;
		case NodeType::BINOP:
			s += "BINOP(";
			s += to_string(*std::get<BinOp>(n.value).left);
			s += ", " + to_string(std::get<BinOp>(n.value).type) + ", ";
			s += to_string(*std::get<BinOp>(n.value).right);
			s += ")";
			break;
		case NodeType::UNRYOP:
			s += "UNRYOP(";
			s += to_string(std::get<UnryOp>(n.value).type) + ", ";
			s += to_string(*std::get<UnryOp>(n.value).right);
			s += ")";
			break;
		case NodeType::ASSIGN:
			s += "ASSIGN(";
			s += std::get<AssignOp>(n.value).var + ", ";
			s += to_string(*std::get<AssignOp>(n.value).expr);
			s += ")";
			break;
		case NodeType::ACCESS:
			s += "ACCESS(";
			s += std::get<std::string>(n.value);
			s += ")";
			break;

		default:
			ASSERT(false, "to_string not defined for this NodeType");
			exit(-1);

		}

#ifdef PRINT_POS
		s += " start: " + to_string(n.start_pos);
		s += ", end: " + to_string(n.end_pos);
#endif

		return s;
	}

	void Parser::advance()
	{
		++m_TokenIndex;

		if (m_TokenIndex < m_Tokens.size())
		{
			m_CurrentToken = &m_Tokens[m_TokenIndex];
		}
	}

	void Parser::retreat()
	{
		--m_TokenIndex;
		if (m_TokenIndex >= 0)
		{
			m_CurrentToken = &m_Tokens[m_TokenIndex];
		}
	}

	ParseResult Parser::atom()
	{
		Token t = *m_CurrentToken;

		switch (t.type)
		{
		case TokenType::INT:
		case TokenType::FLOAT:
		{
			advance();
			return new Node({ NodeType::NUM, t, t.start_pos, t.end_pos });
		}
		case TokenType::ID:
		{
			advance();
			return new Node({ NodeType::ACCESS, std::get<std::string>(t.value), t.start_pos, t.end_pos });
		}
		case TokenType::LROUND:
		{
			advance();
			ParseResult res = expression();
			if (res.index() == (int) ParseRes::ERROR) return res;
			Node* expr = std::get<Node*>(res);

			if (m_CurrentToken->type == TokenType::RROUND)
			{
				advance();
				return { expr };
			}
			else {
				std::string details = "Parser: expected ')' found: " + to_string(t.type);
			Error e = { ErrorType::INVALID_SYNTAX, details, m_CurrentToken->start_pos, m_CurrentToken->end_pos };
			return e;
			}
		}

		default:
		{
			std::string details = "Parser: expected INT, FLOAT, IDENTIFIER, '+', '-', or '(', found: " + to_string(t.type);
			Error e = { ErrorType::INVALID_SYNTAX, details, t.start_pos, t.end_pos };
			return e;
		}
		}
	}

	ParseResult Parser::factor()
	{
		Token t = *m_CurrentToken;

		switch (t.type)
		{
		case TokenType::NOT:
		case TokenType::SUB:
		{
			advance();
			if (m_TokenIndex >= m_Tokens.size()) return Error({ ErrorType::INVALID_SYNTAX, "Parser: Expected Expression found EOF", t.start_pos, t.end_pos });
			auto fac = factor();
			if (fac.index() == (int) ParseRes::ERROR) return fac;
			Node* fac_node = std::get<Node*>(fac);
			Node* n = new Node({ NodeType::UNRYOP, 0, t.start_pos, fac_node->end_pos });
			n->value = UnryOp { t.type, fac_node };
			return n;
		}

		default:
			return callable();
		}

	}

	ParseResult Parser::wrap_callable(Node* node)
	{
		if (m_CurrentToken->type == TokenType::LROUND)
		{
			//TODO
			exit(-1);
		}
		else
		{
			return { node };
		}
	}

	ParseResult Parser::callable()
	{
		auto res = atom();
		if (res.index() == (int) ParseRes::ERROR) return res;
		Node* res_node = std::get<Node*>(res);
		return wrap_callable(res_node);
	}

	ParseResult Parser::term()
	{
		return binop_expression(&Parser::factor, { TokenType::MUL, TokenType::DIV }, &Parser::factor);
	}

	ParseResult Parser::arith_expression()
	{
		return binop_expression(&Parser::term, { TokenType::ADD, TokenType::SUB }, &Parser::term);
	}

	ParseResult Parser::comp_expression()
	{
		return binop_expression(&Parser::arith_expression, 
			{ TokenType::EQUAL, TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ, TokenType::GREATER_EQ },
			&Parser::arith_expression);
	}

	ParseResult Parser::binop_expression(std::function<ParseResult(Parser*)> func_a, std::vector<TokenType> ops,
		std::function<ParseResult(Parser*)> func_b)
	{
		auto left = func_a(this);
		if (left.index() == (int) ParseRes::ERROR) return left;
		Node* left_node = std::get<Node*>(left);
		
		while (std::find(ops.begin(), ops.end(), m_CurrentToken->type) != ops.end())
		{
			Token op_token = *m_CurrentToken;
			advance();

			auto right = func_b(this);
			if (right.index() == (int) ParseRes::ERROR) return right;
			Node* right_node = std::get<Node*>(right);

			Node* node = new Node({ NodeType::BINOP, 0, left_node->start_pos, right_node->end_pos});
			node->value = BinOp { left_node, op_token.type, right_node };
			left_node = node;
		}

		return { left_node };
	}

	ParseResult Parser::expression()
	{
		switch (m_CurrentToken->type)
		{
		case TokenType::ID:
		{
			Token var = *m_CurrentToken;
			advance();

			if (m_CurrentToken->type == TokenType::ASSIGN)
			{
				advance();
				ParseResult res = expression();
				if (res.index() == (int) ParseRes::ERROR) return res;
				return new Node({ NodeType::ASSIGN, AssignOp { std::get<std::string>(var.value), std::get<Node*>(res) } });
			}
			else
			{
				retreat();
				return binop_expression(&Parser::comp_expression, { TokenType::KW_AND, TokenType::KW_OR }, &Parser::comp_expression);
			}
		}
		default:
		{
			return binop_expression(&Parser::comp_expression, { TokenType::KW_AND, TokenType::KW_OR }, &Parser::comp_expression);
		}
		}
		//return binop_expression(&Parser::comp_expression, { TokenType::KW_AND, TokenType::KW_OR }, &Parser::comp_expression);
	}

	ParseResult Parser::parse_nodes()
	{
		if (m_Tokens.empty()) return nullptr;
		return expression();
	}

}
