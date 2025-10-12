#pragma once

#include <string>
#include <vector>
#include <regex>
#include <algorithm>

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
      // @todo João, incompleto: acredito que fazer o match considerando a parte concreta e fazer um sub-match na string
      // restante e fazer backtrack caso não encontre pode funcionar, só precisa achar um match completo para retornar true
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

  // @todo João, mais caracteres para sanitizar aqui....
  pattern = std::regex_replace(pattern, period, "\\.");
  pattern = std::regex_replace(pattern, underscore, ".");
  pattern = std::regex_replace(pattern, percentage, ".*");

  pattern = "^" + pattern + "$";
  // @note João, por hora fiz o teste case insensitive, mas... poderia ser case insentitive e usar uma função 'TO_LOWER'
  // para obter o comportamento desejado, isso quando tiver funções implementadas... Acho que seria melhor, mas por hora
  // fica 'case insensitive' mesmo.
  std::regex like_pattern_regex(pattern, std::regex::icase);
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
  assert(node->op == "like" || node->op == "not like");

  std::string lhs = "";
  std::string rhs = "";
  if (!extract_lhs_and_rhs_expressions(node, table_def, data_row, lhs, rhs))
  {
    return false;
  }
  
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
  else if (node->op == "not like")
  {
    return !evaluate_like_binary_ast_node(node, table_def, data_row);
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

struct Field_Resolver
{
  virtual ~Field_Resolver() = default;

  virtual std::string resolve(std::vector<std::string> &data_row) = 0;
};

struct Field_By_Name_Resolver : Field_Resolver
{
  int64_t index_of_field = -1;

  Field_By_Name_Resolver(CSVData &csv, std::string field_name)
  {
    auto it = std::find(csv.header.begin(), csv.header.end(), field_name);
    
    if (it == csv.header.end())
    {
      assert(false);
      std::cout << "Error: field_name: " << field_name << " não existe no csv." << std::endl;
    }
    
    this->index_of_field = std::distance(csv.header.begin(), it);
  }

  std::string resolve(std::vector<std::string> &data_row)
  {
    assert(this->index_of_field > -1);
    // @note João, não trata a exception in runtime? talvez, talvez fosse melhor retornar string vazia
    return data_row[this->index_of_field];
  }
};

struct String_Literal_Resolver : Field_Resolver
{
  std::string value;

  String_Literal_Resolver(std::string string_value)
  {
    this->value = string_value;
  }

  std::string resolve(std::vector<std::string> &data_row)
  {
    return this->value;
  }
};

bool run_select_on_csv(Select_Ast_Node &select, CSVData &csv)
{
  vector<std::string> new_header;
  vector<Field_Resolver*> field_resolver;
  
  for (auto field : select.fields)
  {
    // @todo João, por hora parseia apenas "ident's" e "string's"
    if (field->type == Ast_Node_Type::Ident_Expression_Ast_Node)
    {
      auto ident = static_cast<Ident_Expression_Ast_Node*>(field.get());
  
      if (ident->ident_name == "*")
      {
        for (auto column : csv.header)
        {
          new_header.push_back(column);
          field_resolver.push_back(new Field_By_Name_Resolver(csv, column));
        }
      }
      else
      {
        if (!contains(csv.header, ident->ident_name))
        {
          std::cout << "Coluna inexistente no dataset: " << ident->ident_name << std::endl;
          return false;
        }
  
        if (ident->as.empty())
        {
          new_header.push_back(ident->ident_name);
        }
        else
        {
          new_header.push_back(ident->as);
        }
        field_resolver.push_back(new Field_By_Name_Resolver(csv, ident->ident_name));
      }
    }
    else if (field->type == Ast_Node_Type::String_Literal_Expression_Ast_Node)
    {
      auto string = static_cast<String_Literal_Expression_Ast_Node*>(field.get());
      if (string->as.empty())
      {
        new_header.push_back(string->value);
      }
      else
      {
        new_header.push_back(string->as);
      }
      field_resolver.push_back(new String_Literal_Resolver(string->value));
    }
    else
    {
      assert(false);
    }
  }

  assert(new_header.size() == field_resolver.size());

  if (csv.dataset.size() == 0)
  {
    for (Field_Resolver* it : field_resolver) delete it;
    
    return false;
  }

  vector<CSV_Data_Row> new_dataset;

  for (CSV_Data_Row &data_row: csv.dataset)
  {
    // @todo João, é necessário validar se o 'comando' faz sentido de acordo com a estrutura da tabela
    if (select.where && select.where->conditions.get())
    {
      if (!evaluate_relational_binary_ast_node(select.where->conditions.get(), &csv.header, &data_row))
      {
        continue;
      }
    }

    std::vector<std::string> new_data_row;
    
    for (auto resolver : field_resolver)
    {
      new_data_row.push_back(resolver->resolve(data_row));
    }

    new_dataset.push_back(new_data_row);
  }

  csv.header = new_header;
  csv.dataset = new_dataset;

  print_as_table(csv, Columns_Print_Mode::All_Columns, NULL, 30);

  for (Field_Resolver* it : field_resolver) delete it;

  return true;
}

bool run_describe_on_csv(Describe_Ast_Node &describe, CSVData &csv)
{
  csv.infer_types();

  CSV_Data_Row header = { "Column Name", "Type", "Nullable" };
  std::vector<CSV_Data_Row> dataset;

  for (size_t i = 0; i < csv.header.size(); i++)
  {
    auto &col_info = csv.infered_types_for_columns.at(i);
    CSV_Data_Row new_row;
    
    new_row.push_back(csv.header.at(i));
    new_row.push_back(to_string(col_info.type));
    new_row.push_back(col_info.nullable ? "yes" : "No");

    dataset.push_back(new_row);
  }

  std::cout << "Describe of table: " << describe.ident_name << std::endl;
  print_as_table(header, dataset, Columns_Print_Mode::All_Columns, NULL, 30);
  
  return false;
}
