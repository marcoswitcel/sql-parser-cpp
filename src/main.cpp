#include <iostream>
#include <string>
#include <vector>

#include "./command-line-utils.cpp"
#include "./utils.cpp"
#include "./sql-parse-context.cpp"
#include "./ast_node.hpp"
#include "./evaluate.cpp"

// @todo João, considerar uma forma mais elegante de fazer esse import
#include "../lib/csv/src/csv.hpp"


int main(int argc, const char* argv[])
{
  if (argc < 2)
  {
    std::cout << "O SQL não foi provido!" << std::endl;
    return EXIT_FAILURE;
  }

  bool is_verbose = is_string_present_in_argv("--verbose", argc, argv);
  bool is_print_tokens = is_string_present_in_argv("--print-tokens", argc, argv);
  Found_Value csv_found = get_value_for_in_argv("--csv-filename", argc, argv);
  // auto filter = get_value_for_in_argv("--sql-command", argc, argv);

  std::string sql_command = std::string(argv[1]);

  if (is_verbose) std::cout << "SQL: " << sql_command << std::endl;
  if (!csv_found.found) std::cout << "Parâmetro --csv-filename é obrigatório." << std::endl;

  SQL_Parse_Context parser(sql_command);

  Ast_Node* node = parser.eat_node();
  
  if (node && node->type == Ast_Node_Type::Select_Ast_Node)
  {
    auto select = dynamic_cast<Select_Ast_Node*>(node);

    auto result = parse_csv_from_file(csv_found.value);

    if (result.first) {
      auto csv = result.second;

      //  checa campos do select
      run_select_on_table(*select, csv.header, csv.dataset);
    }
    else
    {
      // @todo João, colocar aviso aqui...
    }
  }

  if (is_print_tokens)
  {
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
  }

  return EXIT_SUCCESS;
}
