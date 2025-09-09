#include <iostream>
#include <string>
#include <vector>

#include "./command-line-utils.cpp"
#include "./utils.cpp"
#include "./sql-parse-context.cpp"
#include "./ast_node.hpp"

using std::vector;

bool evaluate_relational_binary_ast_node(const Binary_Expression_Ast_Node* node, std::vector<std::string>* table_def, std::vector<std::string>* data_row)
{
  // @todo João, única operação suportada
  assert(node->op == "=");

  assert(node->left->type == Ast_Node_Type::Ident_Expression_Ast_Node ||
    node->left->type == Ast_Node_Type::String_Literal_Expression_Ast_Node);
  assert(node->right->type == Ast_Node_Type::Ident_Expression_Ast_Node ||
    node->right->type == Ast_Node_Type::String_Literal_Expression_Ast_Node);

  std::string lhs = "";
  if (node->left->type == Ast_Node_Type::Ident_Expression_Ast_Node)
  {
    lhs = static_cast<Ident_Expression_Ast_Node*>(node->left.get())->ident_name;
    int64_t index = index_of(*table_def, lhs);
    if (index > -1)
    {
      lhs = data_row->at(index);
    }
    else
    {
      return false;
    }
  }
  else if (node->left->type == Ast_Node_Type::String_Literal_Expression_Ast_Node)
  {
    lhs = static_cast<String_Literal_Expression_Ast_Node*>(node->left.get())->value;
  }
  std::string rhs = "";
  if (node->right->type == Ast_Node_Type::Ident_Expression_Ast_Node)
  {
    rhs = static_cast<Ident_Expression_Ast_Node*>(node->right.get())->ident_name;
    int64_t index = index_of(*table_def, rhs);
    if (index > -1)
    {
      rhs = data_row->at(index);
    }
    else
    {
      return false;
    }
  }
  else if (node->right->type == Ast_Node_Type::String_Literal_Expression_Ast_Node)
  {
    rhs = static_cast<String_Literal_Expression_Ast_Node*>(node->right.get())->value;
  }

  return lhs.compare(rhs) != 0;
}

bool run_select_on_table(Select_Ast_Node &select, std::vector<std::string> &table_def, std::vector<std::vector<std::string>> &table)
{
  vector<size_t> index_to_and_order;

  for (size_t i = 0; i < select.fields.size(); i++)
  {
    Ident_Expression_Ast_Node *ident = select.fields.at(i).get();
    int64_t index = index_of(table_def, ident->ident_name);

    if (index < 0)
    {
      return false;
    }

    index_to_and_order.push_back(static_cast<size_t>(index));
  }

  if (table.size() == 0) return false;;
  
  for (std::vector<std::string> &data_row: table)
  {
    // @note João, inserir alertas?
    if (table_def.size() != data_row.size()) return false;;
  }

  std::cout << "| Printando table |" << std::endl;
  // @todo João, printar header...
  for (std::vector<std::string> &data_row: table)
  {
    // @todo João, implementando um esqueleto de como seria pra interpretar o comando `Name = 'nome-usado'`.
    // Porém, aqui não é o lugar mais apropriado por alguns motivos:
    // * É necessário validar se o 'comando' faz sentido de acordo com a estrutura da tabela
    // * É necessário suportar mais opções de filtros e dessa forma o código ficará enorme...
    if (select.where && select.where->conditions.size())
    {
      if (evaluate_relational_binary_ast_node(select.where->conditions.at(0).get(), &table_def, &data_row))
      {
        continue;
      }
    }

    std::cout << "|";
    for (size_t i : index_to_and_order)
    {
      std::cout << data_row.at(i) << "|";
    }
    std::cout << std::endl;
  }

  return true;
}

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
