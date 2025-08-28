#include <iostream>
#include <string>
#include <algorithm>

#include "./command-line-utils.cpp"
#include "./utils.cpp"
#include "./sql-parse-context.cpp"
#include "./ast_node.hpp"

int main(int argc, const char* argv[])
{
  // @todo João, implementar um sistema mais robusto organizado para lidar com os parâmetros
  if (argc < 2)
  {
    std::cout << "O SQL não foi provido!" << std::endl;
    return EXIT_FAILURE;
  }

  bool is_verbose = is_string_present_in_argv("--verbose", argc, argv);
  // auto filter = get_value_for_in_argv("--sql-command", argc, argv);

  std::string sql_command = std::string(argv[1]);

  if (is_verbose) std::cout << "SQL: " << sql_command << std::endl;

  SQL_Parse_Context parser(sql_command);

  // Select_Ast_Node select;
  // select.fields.push_back(std::make_shared<Ident_Ast_Node>());
  // std::cout << select.to_string() << std::endl;

  parser.skip_whitespace();

  while (!parser.is_finished())
  {
    Token token = parser.eat_token();

    if (parser.error)
    {
      std::cout << "Error: tokenização não terminou" << std::endl;
      break;
    }
  
    parser.skip_whitespace();
    
    std::cout << token.to_string() << std::endl;
  }

  return EXIT_SUCCESS;
}
