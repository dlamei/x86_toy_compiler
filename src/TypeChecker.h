#pragma once

#include "Parser.h"
#include "Debug.h"

namespace Chronos
{

	class TypeChecker
	{
	private:
		uint32_t m_IntCount = 0;
		uint32_t m_FloatCount = 0;
		uint32_t m_PtrCount = 0;

		ValueType check_type_unryop(NodeValues::UnryOp& op);
		ValueType check_type_assign(NodeValues::AssignOp& op);
		ValueType check_type_access(std::string var);
		ValueType check_type_num(TokenType type);
		ValueType check_type_binop(NodeValues::BinOp& binop);
		ValueType check_type_arith_binop(NodeValues::BinOp& binop);
		ValueType check_type_logic_binop(NodeValues::BinOp& binop);

	public:
		inline uint32_t get_int_count() { return m_IntCount; }
		inline uint32_t get_float_count() { return m_FloatCount; }
		inline uint32_t get_ptr_count() { return m_PtrCount; }

		uint32_t get_alloc_size() { return 4 * (m_IntCount + m_FloatCount + m_PtrCount); }

		ValueType check_type(Node* node);
	};
}
