#pragma once

#include <string>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <ctime>
#include <vector>
#include <chrono>
#include <regex>
#include <algorithm>

#include "./utils.cpp"
#include "./ast_node.hpp"
#include "./aggregator.cpp"
#include "./collector_ast_node_visitor.hpp"
#include "./resolver.cpp"
#include "./ordered_map.hpp"
// Dependências
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
  Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row,
  std::string &lhs, std::string &rhs)
{
  Expression_Resolver resolver_left = Expression_Resolver(&csv.header, node->left.get());
  Expression_Resolver resolver_right = Expression_Resolver(&csv.header, node->right.get());

  lhs = resolver_left.resolve(data_row);
  rhs = resolver_right.resolve(data_row);

  return true;
}

bool evaluate_equals_binary_ast_node(Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row)
{
  assert(node->op == "=" || node->op == "<>");

  std::string lhs = "";
  std::string rhs = "";
  if (!extract_lhs_and_rhs_expressions(node, csv, data_row, lhs, rhs))
  {
    return false;
  }

  // @todo João, não lida com números, possivelmente se aplica em outras sessões também
  return lhs.compare(rhs) == 0;
}

float evaluate_compare_binary_ast_node(Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row)
{
  assert(node->op == ">" || node->op == "<");

  std::string lhs = "";
  std::string rhs = "";
  if (!extract_lhs_and_rhs_expressions(node, csv, data_row, lhs, rhs))
  {
    return 0;
  }

  // @todo João, completamente errado... precisa considerar strings inválidas e principalmente, precisa retornar os números diretamente
  return std::stof(lhs) - std::stof(rhs);
}

bool evaluate_not_equals_binary_ast_node(Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row)
{
  return !evaluate_equals_binary_ast_node(node, csv, data_row);
}

bool evaluate_like_binary_ast_node(Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row)
{
  assert(node->op == "like" || node->op == "not like");

  std::string lhs = "";
  std::string rhs = "";
  if (!extract_lhs_and_rhs_expressions(node, csv, data_row, lhs, rhs))
  {
    return false;
  }
  
  return run_like_pattern_on(lhs, rhs);
}

bool evaluate_relational_binary_ast_node(Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row)
{
  if (node->op == "=")
  {
    return evaluate_equals_binary_ast_node(node, csv, data_row);
  }
  else if (node->op == "<")
  {
    return evaluate_compare_binary_ast_node(node, csv, data_row) < 0;
  }
  else if (node->op == ">")
  {
    return evaluate_compare_binary_ast_node(node, csv, data_row) > 0;
  }
  else if (node->op == "<>")
  {
    return evaluate_not_equals_binary_ast_node(node, csv, data_row);
  }
  else if (node->op == "like")
  {
    return evaluate_like_binary_ast_node(node, csv, data_row);
  }
  else if (node->op == "not like")
  {
    return !evaluate_like_binary_ast_node(node, csv, data_row);
  }
  else if (node->op == "or")
  {
    return evaluate_relational_binary_ast_node(static_cast<Binary_Expression_Ast_Node *>(node->left.get()), csv, data_row) ||
      evaluate_relational_binary_ast_node(static_cast<Binary_Expression_Ast_Node *>(node->right.get()), csv, data_row);
  }
  else if (node->op == "and")
  {
    return evaluate_relational_binary_ast_node(static_cast<Binary_Expression_Ast_Node *>(node->left.get()), csv, data_row) &&
      evaluate_relational_binary_ast_node(static_cast<Binary_Expression_Ast_Node *>(node->right.get()), csv, data_row);
  }

  // @todo João, por hora o makefile faz cair em uma das de cima
  assert(false);
  return false;
}



bool does_field_exist(CSVData &csv, std::string field_name)
{
  auto it = std::find(csv.header.begin(), csv.header.end(), field_name);
  
  return it != csv.header.end();
}

bool run_select_on_csv(Select_Ast_Node &select, CSVData &csv)
{

  Collector_Ast_Node_Visitor collector;

  select.accept(collector);

  for (auto field : collector.idents)
  {
    if (field == "*") continue;

    if (!does_field_exist(csv, field))
    {
      std::cout << "Error: field_name: " << field << " não existe no csv." << std::endl;
      return false;
    }
  }

  vector<std::string> new_header;
  vector<Field_Resolver*> field_resolver;
  
  // @todo João ainda falta validação semântica dos campos...
  for (auto field : select.fields)
  {
    field->infer_type();
    
    if (field->type == Ast_Node_Type::Ident_Expression_Ast_Node)
    {
      auto ident = static_cast<Ident_Expression_Ast_Node*>(field.get());
  
      if (ident->ident_name == "*")
      {
        for (auto column : csv.header)
        {
          new_header.push_back(column);
          field_resolver.push_back(new Field_By_Name_Resolver(csv.header, column));
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
        field_resolver.push_back(new Field_By_Name_Resolver(csv.header, ident->ident_name));
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
    else if (field->type == Ast_Node_Type::Number_Literal_Expression_Ast_Node)
    {
      auto number = static_cast<Number_Literal_Expression_Ast_Node*>(field.get());
      if (number->as.empty())
      {
        new_header.push_back(std::to_string(number->value));
      }
      else
      {
        new_header.push_back(number->as);
      }
      field_resolver.push_back(new Number_Literal_Resolver(number->value));
    }
    else if (field->type == Ast_Node_Type::Binary_Expression_Ast_Node && static_cast<Binary_Expression_Ast_Node*>(field.get())->op == "concat")
    {
      auto bin_expr = static_cast<Binary_Expression_Ast_Node*>(field.get());
      if (bin_expr->as.empty())
      {
        new_header.push_back(bin_expr->to_expression());
      }
      else
      {
        new_header.push_back(bin_expr->as);
      }
      field_resolver.push_back(new Binary_Expression_Resolver(&csv.header, bin_expr));
    }
    else if (field->type == Ast_Node_Type::Function_Call_Expression_Ast_Node && known_function_name_and_argument_list(static_cast<Function_Call_Expression_Ast_Node*>(field.get())))
    {
      auto call_expr = static_cast<Function_Call_Expression_Ast_Node*>(field.get());
      if (call_expr->as.empty())
      {
        new_header.push_back(call_expr->to_expression());
      }
      else
      {
        new_header.push_back(call_expr->as);
      }
      // @todo joão, falta validar idents...
      field_resolver.push_back(new Function_Call_Expression_Resolver(&csv.header, call_expr));
    }
    else
    {
      std::cout << "A expressão a seguir não pode ser interpretada: " << std::endl << field->to_expression() << std::endl;
      return false;
    }
  }

  assert(new_header.size() == field_resolver.size());

  if (csv.dataset.size() == 0)
  {
    for (Field_Resolver* it : field_resolver) delete it;
    
    return false;
  }


  const auto hasWhere = select.where && select.where->conditions.get();
  const auto hasGroupBy = select.group_by && select.group_by->groups.size() > 0;
  vector<CSV_Data_Row> new_dataset;
  std::unique_ptr<Aggregator> root_aggregator;

  if (hasGroupBy)
  {
    vector<std::unique_ptr<Field_By_Name_Resolver>> field_by_name_resolvers;
    
    for (auto &field : select.fields)
    {
      if (auto ident = Cast_If(Ident_Expression_Ast_Node, *field))
      {
        bool found = false;

        for (auto &grouping_field : select.group_by->groups)
        {
          if (auto group_by_ident = Cast_If(Ident_Expression_Ast_Node, *grouping_field))
          {
            if (group_by_ident->ident_name == ident->ident_name)
            {
              field_by_name_resolvers.push_back(std::make_unique<Field_By_Name_Resolver>(csv.header, ident->ident_name));
              found = true;
              continue;
            } 
          }
          else
          {
            std::cout << "Por hora todas as expressões no Group By precisam ser identificadores simples." << std::endl;
            return false;
          }
        }
        
        if (!found)
        {
          std::cout << "Por hora todos os campos do select precisam estar contidos na cláusula Group By." << std::endl;
          return false;
        }
      }
      else
      {
        std::cout << "Por hora todos os campos do select precisam ser compostos apenas por identificadores." << std::endl;
        return false;
      }
    }
    
    // montando estrutura de agregadores
    for (size_t i = select.group_by->groups.size(); i > 0; i--)
    {
      std::unique_ptr<Expression_Ast_Node> &grouping_field = select.group_by->groups.at(i - 1);

      if (auto ident = Cast_If(Ident_Expression_Ast_Node, *grouping_field))
      {
        auto &field_name = ident->ident_name;
        auto field_resolver = std::make_unique<Field_By_Name_Resolver>(csv.header, field_name);
        
        if (root_aggregator)
        {
          auto aggregator = std::make_unique<Subgrouping_Aggregator>(field_resolver, root_aggregator);
          root_aggregator = std::move(aggregator);
        }
        else
        {
          auto aggregator = std::make_unique<Value_Aggregator>(field_resolver);
          root_aggregator = std::move(aggregator);
        }
      }
      else
      {
        std::cout << "A expressão a seguir não pode ser aplicada no Group By: " << std::endl << grouping_field->to_expression() << std::endl;
        return false;
      }
    }
    
    // executando processo de agregação
    for (CSV_Data_Row &data_row: csv.dataset)
    {
      if (hasWhere)
      {
        if (!evaluate_relational_binary_ast_node(select.where->conditions.get(), csv, data_row))
        {
          continue;
        }
      }
      
      root_aggregator->aggregate(&data_row);
    }

    for (auto &field_by_name_resolver : field_by_name_resolvers)
    {
      // @todo João, temporario
      field_by_name_resolver->index_of_field = 0;
    }
    
    // @todo João, @wip terminar aqui... a ideia é começar implementando o count(*), select species From Iris Group By Species
    while (auto value = root_aggregator->get_next_group_value())
    {
      std::vector<std::string> new_data_row;

      for (auto &resolver : field_by_name_resolvers)
      {
        new_data_row.push_back(resolver->resolve(value->first));
      }
  
      new_dataset.push_back(new_data_row);
    }
  }
  else
  {
    // caminho rápido quando não há agregador
    for (CSV_Data_Row &data_row: csv.dataset)
    {
      // @todo João, é necessário validar se o 'comando' faz sentido de acordo com a estrutura da tabela
      if (hasWhere)
      {
        if (!evaluate_relational_binary_ast_node(select.where->conditions.get(), csv, data_row))
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

  std::cout << "Describe of table: " << describe.ident_name->to_expression() << std::endl;
  print_as_table(header, dataset, Columns_Print_Mode::All_Columns, NULL, 30);
  
  return false;
}
