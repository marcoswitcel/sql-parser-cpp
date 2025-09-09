#include <iostream>
#include <string>
#include <vector>

#include "./command-line-utils.cpp"
#include "./utils.cpp"
#include "./sql-parse-context.cpp"
#include "./ast_node.hpp"
#include "./evaluate.cpp"


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

  Ast_Node* node = parser.eat_node();
  
  if (node && node->type == Ast_Node_Type::Select_Ast_Node)
  {
    auto select = dynamic_cast<Select_Ast_Node*>(node);
    // std::cout << select->to_string() << std::endl;

    std::vector<std::string> table_def = {"Phone1", "Phone2", "Name"};
    std::vector<std::vector<std::string>> table_data;

    table_data.push_back({"55 9xxxx-9999", "(55) 9xxxx-9999", "Marcelson"});
    table_data.push_back({"55 9xxxx-8888", "(55) 9xxxx-8888", "Jucelson"});

    //  checa campos do select
    run_select_on_table(*select, table_def, table_data);
  }

  parser = SQL_Parse_Context(sql_command);

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
