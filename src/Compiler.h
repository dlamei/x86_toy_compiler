#pragma once

#include <fstream>
#include <unordered_map>
#include <string>
#include <stack>

#include "Parser.h"
#include "Debug.h"
#include "TypeChecker.h"

#define HEADER_SIZE 4
#define PTR_SIZE 4

namespace Chronos
{

	namespace x86ASM
	{
		using Label = const char*;
		struct SubLabel
		{
			uint32_t count = 0;
		};

		enum Section : uint8_t
		{
			DATA = 0,
			BSS,
			TEXT,
			NO_SECTION,
		};

		enum InstType : uint16_t
		{
			#define INST_TYPE(a) a,
			#define REGISTER(a)
			#include "x86ASM.h"
		};

		enum class Reg : uint8_t
		{
			#define INST_TYPE(a)
			#define REGISTER(a) a,
			#include "x86ASM.h"
		};

		enum DerefSize : uint8_t
		{
			BYTE = 0,
			WORD,
			DWORD,
			QWORD,
			NO_DEREF,
		};

		enum ReserveSize : uint8_t
		{
			RESB = 0,
			RESW,
			RESQ,
			NO_RESERVE,
		};

		enum DefineSize : uint8_t
		{
			DB = 0,
			DW,
			DQ,
			NO_DEFINE,
		};

		enum MemAdressType
		{
			REGISTER = 0,
			LABEL_ADR,
			SUB_LABEL_ADR,
		};

		using MemAdress = std::variant<Reg, const char*, SubLabel>;

		struct MemAccess
		{
			MemAdress adress = Reg::NO_REG;
			int offset = 0;
			DerefSize size = NO_DEREF;

			MemAccess(const char* adr)
				: adress(adr) {}
			MemAccess(SubLabel l)
				: adress(l) {}
			MemAccess(Reg reg)
				: adress(reg) {}
			MemAccess(Reg reg, int off)
				: adress(reg), offset(off) {}
			MemAccess(const char* adr, int off)
				: adress(adr), offset(off) {}
			MemAccess(Reg reg, int off, DerefSize s)
				: adress(reg), offset(off), size(s) {}
			MemAccess(const char* reg, int off, DerefSize s)
				: adress(reg), offset(off), size(s) {}
		};

		struct ReserveMem
		{
			const char* name;
			ReserveSize size = NO_RESERVE;
			int count;
		};


		struct DefineMem
		{
			const char* name;
			DefineSize size = NO_DEFINE;
			std::vector<std::variant<const char*, int>> bytes;
		};


		static const int MEM_ACCESS = 0;
		static const int INT_VALUE = 1;
		static const int FLOAT_VALUE = 2;
		static const int NO_ADR = 3;

		struct BasicInst
		{
			InstType type;
			std::variant<MemAccess, int, float, bool> adresses[2] = { false, false };

		};

		enum InstructionType : uint8_t
		{
			BASIC_INST = 0,
			RESERVE_MEM,
			DEFINE_MEM,
			SECTION,
			SUB_LABEL,
		};

		using Instruction = std::variant<BasicInst, ReserveMem, DefineMem, Section, SubLabel>;
	}

	std::string to_string(std::unordered_map<x86ASM::Label, std::vector<x86ASM::Instruction>>& m_Code);
	using ASMCode = std::unordered_map<x86ASM::Label, std::vector<x86ASM::Instruction>>;

	struct StackVal
	{
		int offset;
		ValueType type;
	};

	class Compiler
	{
	private:
		const char* m_Name = "";

		x86ASM::Label m_CurrentLabel = "";
		ASMCode m_Code;

		std::unordered_map<std::string, StackVal> m_VarTable; //<name, offset>
		int m_BPOffset = 4;
		uint32_t m_CurrentSubLabel = 0;

		std::ofstream m_Output;

		void set_label(x86ASM::Label l) { m_CurrentLabel = l; }
		x86ASM::SubLabel sub_label();
		x86ASM::SubLabel sub_label(uint32_t offset); 
		void offset_sub_label(uint32_t offset); 
		void write(x86ASM::Instruction i);
		void write(x86ASM::InstType t, x86ASM::MemAccess a);
		void write(x86ASM::InstType t, x86ASM::MemAccess a, x86ASM::MemAccess b);
		void write(x86ASM::InstType t, x86ASM::MemAccess a, int b);
		void write(x86ASM::InstType t, x86ASM::MemAccess a, float b);
		void write(x86ASM::InstType t, int a);
		void write(x86ASM::InstType t, float a);
		void write_section(x86ASM::Section s);
		void write_mem_def(const char* var, x86ASM::DefineSize size, std::vector<std::variant<const char*, int>> bytes);
		void write_mem_res(const char* var, x86ASM::ReserveSize size, int count);

		void zero_cmp_float();
		void zero_cmp_int();

		void print_top();
		void print_chint();
		void print_value(ValueType type);

		void eval_num(Token& token);
		void int_int_binop(TokenType type);
		void float_float_binop(TokenType type);
		void eval_arith_binop(Node* node);
		void eval_AND_binop(Node* node);
		void eval_OR_binop(Node* node);
		void float_float_CMP(NodeValues::BinOp& op);
		void int_int_CMP(NodeValues::BinOp& op);
		void eval_CMP_binop(Node* node);
		void eval_binop(Node* node);
		void eval_SUB_unryop(Node* node);
		void eval_NOT_unryop(Node* node);
		void eval_unryop(Node* node);
		void eval_assing(Node* node);
		void eval_expr(Node* node);
		void eval_access(std::string var);


	public:
		void compile(const char* name, Node* node);
		void close();

		~Compiler()
		{
			close();
		}
	};


}
