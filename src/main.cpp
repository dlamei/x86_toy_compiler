#include <iostream>
#include <string>
#include <cstdint>


#include "Lexer.h"
#include "Parser.h"
#include "Compiler.h"

extern "C"
{
#include "chlib.h"
}


int main()
{
	//ch_heap* h = alloc_heap();

	//ch_int* ptr = heap_alloc_int(h, 3);
	//if (ptr)
	//{
	//	print_line_marks(h->blocks);
	//	std::cout << type_from_ptr(h->blocks, ptr) << "\n";
	//}

	//std::cin.get();

	Chronos::FileManager fm;

	std::string buffer;
	Chronos::Lexer lexer;
	Chronos::Parser parser;
	Chronos::Compiler compiler;

	Chronos::NodeValues::Root nodes;
	Chronos::Node* root = new Chronos::Node({ Chronos::NodeType::ROOT,  nodes });

	while (true)
	{
		printf("chronos > ");
		std::getline(std::cin, buffer);

		if (buffer == "exit") break;

		fm.add_line(buffer, "<STDIN>");

		lexer.load_text(buffer.c_str(), buffer.size());
		lexer.parse_tokens();
		lexer.print_tokens();

		if (lexer.has_error())
		{
			std::cout << lexer.get_error().generate_message(fm.get_files()) << "\n";
		}

		parser.load_tokens(lexer.get_tokens());
		Chronos::ParseResult res = parser.parse_nodes();

		if (res.index() == (int) Chronos::ParseRes::OK)
		{
			Chronos::Node* node = std::get<Chronos::Node*>(res);
			if (node) std::cout << "result: " << Chronos::to_string(*node) << "\n";
			std::get<Chronos::NodeValues::Root>(root->value).nodes.push_back(node);
			//nodes.push_back(node);
			//compiler.compile("Chronos", nodes);

		}
		else std::cout << "error: " << std::get<Chronos::Error>(res).generate_message(fm.get_files()) << "\n";

		lexer.clear();
		fm.clear();
	}

	compiler.compile("Chronos", root);

	compiler.close();
	Chronos::delete_nodes(root);
}
