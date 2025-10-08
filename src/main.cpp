#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

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

  bool is_verbose = is_string_present_in_argv("--verbose", argc, argv);
  bool is_print_tokens = is_string_present_in_argv("--print-tokens", argc, argv);
  Found_Value csv_found = get_value_for_in_argv("--csv-filename", argc, argv);
  Found_Value bind_defs_found = get_value_for_in_argv("--bind", argc, argv);

  std::string sql_command = std::string(argv[1]);
  std::unordered_map<std::string, std::string> table_binds;

  if (is_verbose) std::cout << "SQL: " << sql_command << std::endl;

  SQL_Parse_Context parser(sql_command);

  Ast_Node* node = parser.eat_node();
  
  if (bind_defs_found.found)
  {
    for (auto bind : split_by(bind_defs_found.value, ','))
    {
      std::vector<std::string> bind_splited = split_by(bind, '=');
      if (bind_splited.size() != 2)
      {
        std::cout << "Argumento inválido: " << bind << std::endl;
      }
      else
      {
        // insere e ou atualiza em caso de valores duplicados
        table_binds[bind_splited[0]] = bind_splited[1];
      }
    }
  }

  if (csv_found.found)
  {
    // insere e ou atualiza...
    table_binds["csv_file"] = std::string(csv_found.value);
  }

  if (node && node->type == Ast_Node_Type::Select_Ast_Node)
  {
    auto select = dynamic_cast<Select_Ast_Node*>(node);
    auto table_name = select->from->ident_name;
    
    if (table_binds.count(table_name) > 0)
    {
      auto filename = table_binds.at(table_name);
      auto result = parse_csv_from_file(filename.c_str());
  
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
    else
    {
      if (is_verbose) std::cout << "Bind para tabela '" << table_name << "' não encontrado. " << std::endl;
    }
  }
  else if (node && node->type == Ast_Node_Type::Describe_Ast_Node)
  {
    auto describe = dynamic_cast<Describe_Ast_Node*>(node);
    auto table_name = describe->ident_name;

    if (table_binds.count(table_name) > 0)
    {
      auto filename = table_binds.at(table_name);
  
      auto result = parse_csv_from_file(filename.c_str());
  
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
      if (is_verbose) std::cout << "Bind para tabela '" << table_name << "' não encontrado. " << std::endl;
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
