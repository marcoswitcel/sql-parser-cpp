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
  bool is_help = is_string_present_in_argv("--help", argc, argv);
  if (is_help)
  {
    std::cout << "<program> <comando sql> --argumentos-opcionais" << std::endl;
    return EXIT_FAILURE;
  }

  if (argc < 2)
  {
    std::cout << "O comando SQL não foi provido!" << std::endl;
    return EXIT_FAILURE;
  }

  // @todo João, revisar parâmetros e forma de aplicação, acho que faz sentido sql sem --csv-filename em função do print-tokens
  bool is_verbose = is_string_present_in_argv("--verbose", argc, argv);
  bool is_print_tokens = is_string_present_in_argv("--print-tokens", argc, argv);
  Found_Value csv_found = get_value_for_in_argv("--csv-filename", argc, argv);
  // @wip @todo João, terminar de implementar o bind de símbolos (table names) para arquivos csv
  // Found_Value bind_defs_found = get_value_for_in_argv("--bind", argc, argv);

  std::string sql_command = std::string(argv[1]);

  if (is_verbose) std::cout << "SQL: " << sql_command << std::endl;
  if (!csv_found.found) std::cout << "Parâmetro --csv-filename é obrigatório." << std::endl;

  SQL_Parse_Context parser(sql_command);

  Ast_Node* node = parser.eat_node();
  
  if (node && node->type == Ast_Node_Type::Select_Ast_Node)
  {
    const char* filename = csv_found.value;
    auto select = dynamic_cast<Select_Ast_Node*>(node);

    auto result = parse_csv_from_file(filename);

    if (result.first) {
      auto csv = result.second;

      if (csv.parsing_errors.size())
      {
        if (is_verbose) std::cout << "Encontrou erros ao parsear o CSV: " << filename << std::endl;  
      }
      else
      {
        //  checa campos do select
        run_select_on_csv(*select, csv);
      }

    }
    else
    {
      if (is_verbose) std::cout << "arquivo não encontrado: " << filename << std::endl;
    }
  }
  else if (node && node->type == Ast_Node_Type::Describe_Ast_Node)
  {
    const char* filename = csv_found.value;
    auto describe = dynamic_cast<Describe_Ast_Node*>(node);

    auto result = parse_csv_from_file(filename);

    if (result.first) {
      auto csv = result.second;

      if (csv.parsing_errors.size())
      {
        if (is_verbose) std::cout << "Encontrou erros ao parsear o CSV: " << filename << std::endl;  
      }
      else
      {
        //  checa campos do describe
        run_describe_on_csv(*describe, csv);
      }

    }
    else
    {
      if (is_verbose) std::cout << "arquivo não encontrado: " << filename << std::endl;
    }
  }
  else
  {
    if (is_verbose) std::cout << "Não conseguiu parsear o comando: " << sql_command << std::endl;
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
