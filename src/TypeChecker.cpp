#include "TypeChecker.h"

namespace Chronos
{
	using namespace NodeValues;

	std::unordered_map<std::string, ValueType> Scope;

	ValueType TypeChecker::check_type_num(TokenType type)
	{
		switch (type)
		{
		case TokenType::INT:
			return ValueType::INT;
		case TokenType::FLOAT:
			return ValueType::FLOAT;
		default:
			return ValueType::NONE;
		}
	}

	ValueType TypeChecker::check_type_logic_binop(BinOp& binop)
	{
		check_type(binop.left);
		check_type(binop.right);

		return ValueType::INT;
	}

	ValueType TypeChecker::check_type_arith_binop(BinOp& binop)
	{
		ValueType ltype = check_type(binop.left);
		ValueType rtype = check_type(binop.right);

		if (ltype == ValueType::POINTER || rtype == ValueType::POINTER) return ValueType::POINTER;
		if (ltype == ValueType::FLOAT || rtype == ValueType::FLOAT) return ValueType::FLOAT;
		if (ltype == ValueType::INT || rtype == ValueType::INT) return ValueType::INT;
		if ((ltype == ValueType::INT && rtype == ValueType::FLOAT) || (ltype == ValueType::FLOAT && rtype == ValueType::FLOAT)) return ValueType::FLOAT;
		else return ValueType::POINTER;
	}

	ValueType TypeChecker::check_type_binop(BinOp& binop)
	{
		switch (binop.type)
		{
		case TokenType::KW_AND:
		case TokenType::KW_OR:
		case TokenType::EQUAL:
		case TokenType::LESS:
		case TokenType::LESS_EQ:
		case TokenType::GREATER:
		case TokenType::GREATER_EQ:
			return check_type_logic_binop(binop);
		case TokenType::ADD:
		case TokenType::SUB:
		case TokenType::MUL:
		case TokenType::DIV:
			return check_type_arith_binop(binop);

		default:
			ASSERT(false, "this type of binop is not supported");
			exit(-1);
		}
	}

	ValueType TypeChecker::check_type_assign(AssignOp& op)
	{
		ValueType type = check_type(op.expr);

		switch (type)
		{
		case ValueType::INT:
			++m_IntCount;
			break;
		case ValueType::FLOAT:
			++m_FloatCount;
			break;
		case ValueType::POINTER:
			++m_PtrCount;
			break;
		}

		//TODO: what is with reassign
		Scope.emplace(std::make_pair(op.var, type));
		return type;
	}

	ValueType TypeChecker::check_type_access(std::string var)
	{
		//TODO: check if exists
		return Scope.at(var);
	}

	ValueType TypeChecker::check_type_unryop(UnryOp& op)
	{
		check_type(op.right);
		if (op.type == TokenType::NOT) return ValueType::INT;
		return check_type(op.right);
	}

	ValueType TypeChecker::check_type(Node* node)
	{
		if (!node) return ValueType::NONE;

		switch (node->type)
		{
		case NodeType::ROOT:
			node->value_type = ValueType::NONE;
			for (Node* n : std::get<Root>(node->value).nodes) check_type(n);
			break;

		case NodeType::NUM:
			node->value_type = check_type_num(std::get<Token>(node->value).type);
			break;
		case NodeType::BINOP:
			node->value_type = check_type_binop(std::get<BinOp>(node->value));
			break;
		case NodeType::ACCESS:
			node->value_type = check_type_access(std::get<std::string>(node->value));
			break;
		case NodeType::ASSIGN:
			node->value_type = check_type_assign(std::get<AssignOp>(node->value));
			break;
		case NodeType::UNRYOP:
			node->value_type = check_type_unryop(std::get<UnryOp>(node->value));
			break;
		}

		return node->value_type;
	}

}
