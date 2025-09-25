#pragma once

#include <string>
#include <vector>
#include <regex>

#include "./utils.cpp"
#include "./ast_node.hpp"

#include "../lib/csv/src/csv.hpp"


using std::vector;

bool run_like_pattern_on_done_manually(std::string text_input, std::string like_pattern)
{
  size_t input_index = 0;
  size_t pattern_index = 0;

  while (input_index < text_input.size())
  {
    if (pattern_index >= like_pattern.size()) return false;

    auto pattern_char = like_pattern.at(pattern_index);
    auto text_char = text_input.at(input_index);
    if (pattern_char == '%')
    {
      if (pattern_index + 1 == like_pattern.size())
      {
        return true;
      }
      // @todo João, incompleto
      return false;
    }
    else if (pattern_char == '_')
    {
      input_index++;
      pattern_index++;
    }
    else
    {
      if (pattern_char != text_char) return false;
      input_index++;
      pattern_index++;
    }
  }

  return input_index >= text_input.size() && pattern_index >= like_pattern.size();
}

bool run_like_pattern_on(std::string text_input, std::string raw_like_pattern)
{
  static std::regex percentage("%");
  static std::regex underscore("_");
  static std::regex period("\\.");

  std::string pattern = raw_like_pattern;

  pattern = std::regex_replace(pattern, period, "\\.");
  pattern = std::regex_replace(pattern, underscore, ".");
  pattern = std::regex_replace(pattern, percentage, ".*");

  pattern = "^" + pattern + "$";
  std::regex like_pattern_regex(pattern);
  return std::regex_match(text_input, like_pattern_regex);
}

bool extract_lhs_and_rhs_expressions(
  const Binary_Expression_Ast_Node* node, std::vector<std::string>* table_def,
  std::vector<std::string>* data_row, std::string &lhs, std::string &rhs)
{
  assert(node->left->type == Ast_Node_Type::Ident_Expression_Ast_Node ||
    node->left->type == Ast_Node_Type::String_Literal_Expression_Ast_Node);
  assert(node->right->type == Ast_Node_Type::Ident_Expression_Ast_Node ||
    node->right->type == Ast_Node_Type::String_Literal_Expression_Ast_Node);

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

  return true;
}

bool evaluate_equals_binary_ast_node(const Binary_Expression_Ast_Node* node, std::vector<std::string>* table_def, std::vector<std::string>* data_row)
{
  assert(node->op == "=" || node->op == "<>");

  std::string lhs = "";
  std::string rhs = "";
  if (!extract_lhs_and_rhs_expressions(node, table_def, data_row, lhs, rhs))
  {
    return false;
  }

  return lhs.compare(rhs) == 0;
}

bool evaluate_not_equals_binary_ast_node(const Binary_Expression_Ast_Node* node, std::vector<std::string>* table_def, std::vector<std::string>* data_row)
{
  return !evaluate_equals_binary_ast_node(node, table_def, data_row);
}

bool evaluate_like_binary_ast_node(const Binary_Expression_Ast_Node* node, std::vector<std::string>* table_def, std::vector<std::string>* data_row)
{
  assert(node->op == "like");

  std::string lhs = "";
  std::string rhs = "";
  if (!extract_lhs_and_rhs_expressions(node, table_def, data_row, lhs, rhs))
  {
    return false;
  }
  // @todo João, terminar de implementar o like
  // Uma das ideias é converter para uma regex, tipo '%joao%' viraria '/.*joao.*/', algo assim
  return run_like_pattern_on(lhs, rhs);
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
  else if (node->op == "like")
  {
    return evaluate_like_binary_ast_node(node, table_def, data_row);
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
