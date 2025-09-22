#pragma once

#include <string>
#include <vector>

#include "./utils.cpp"
#include "./ast_node.hpp"

#include "../lib/csv/src/csv.hpp"


using std::vector;

bool evaluate_equals_binary_ast_node(const Binary_Expression_Ast_Node* node, std::vector<std::string>* table_def, std::vector<std::string>* data_row)
{
  assert(node->op == "=" || node->op == "<>");

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

  return lhs.compare(rhs) == 0;
}

bool evaluate_not_equals_binary_ast_node(const Binary_Expression_Ast_Node* node, std::vector<std::string>* table_def, std::vector<std::string>* data_row)
{
  return !evaluate_equals_binary_ast_node(node, table_def, data_row);
}

bool evaluate_relational_binary_ast_node(const Binary_Expression_Ast_Node* node, std::vector<std::string>* table_def, std::vector<std::string>* data_row)
{
  if (node->op == "=")
  {
    return evaluate_equals_binary_ast_node(node, table_def, data_row);
  }
  else if (node->op == "<>")
  {
    return evaluate_not_equals_binary_ast_node(node, table_def, data_row);
  }
  else if (node->op == "or")
  {
    return evaluate_relational_binary_ast_node(static_cast<const Binary_Expression_Ast_Node *>(node->left.get()), table_def, data_row) ||
      evaluate_relational_binary_ast_node(static_cast<const Binary_Expression_Ast_Node *>(node->right.get()), table_def, data_row);
  }
  else if (node->op == "and")
  {
    return evaluate_relational_binary_ast_node(static_cast<const Binary_Expression_Ast_Node *>(node->left.get()), table_def, data_row) &&
      evaluate_relational_binary_ast_node(static_cast<const Binary_Expression_Ast_Node *>(node->right.get()), table_def, data_row);
  }

  // @todo João, por hora o makefile faz cair em uma das de cima
  assert(false);
  return false;
}

bool run_select_on_csv(Select_Ast_Node &select, CSVData &csv)
{
  vector<std::string> columns;
  
  for (auto ident : select.fields)
  {
    columns.push_back(ident->ident_name);
  }

  for (auto column : columns)
  {
    if (!contains(csv.header, column))
    {
      std::cout << "Coluna inexistente no dataset: " << column << std::endl;
      return false;
    }
  }

  if (csv.dataset.size() == 0) return false;

  vector<CSV_Data_Row> new_dataset;

  for (CSV_Data_Row &data_row: csv.dataset)
  {
    // @todo João, é necessário validar se o 'comando' faz sentido de acordo com a estrutura da tabela
    // @todo João, é necessário suportar mais opções de filtros e dessa forma o código ficará enorme...
    if (select.where && select.where->conditions.get())
    {
      if (!evaluate_relational_binary_ast_node(select.where->conditions.get(), &csv.header, &data_row))
      {
        continue;
      }
    }

    new_dataset.push_back(data_row);
  }

  csv.dataset = new_dataset;

  print_as_table(csv, Columns_Print_Mode::Included_And_Ordered_Columns, &columns, 30);

  return true;
}
