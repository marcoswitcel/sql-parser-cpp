#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include <chrono>

#include "./ast_node.hpp"
#include "./resolver.hpp"

#include "../lib/csv/src/csv.hpp"


Field_By_Name_Resolver::Field_By_Name_Resolver(Tabular_Data_Header &header, std::string field_name)
{
  auto it = std::find(header.begin(), header.end(), field_name);
  
  if (it == header.end())
  {
    assert(false);
    std::cout << "Error: field_name: " << field_name << " não existe no csv." << std::endl;
  }
  
  this->index_of_field = std::distance(header.begin(), it);
  this->field_name = field_name;
}

std::string Field_By_Name_Resolver::resolve(Tabular_Data_Row &data_row)
{
  assert(this->index_of_field > -1);
  // @note João, não trata a exception in runtime? talvez, talvez fosse melhor retornar string vazia
  return data_row[this->index_of_field];
}

std::string String_Literal_Resolver::resolve([[maybe_unused]] Tabular_Data_Row &data_row)
{
  return this->value;
}

std::string Number_Literal_Resolver::resolve([[maybe_unused]] Tabular_Data_Row &data_row)
{
  return std::to_string(this->value);
}

Expression_Resolver::Expression_Resolver(Tabular_Data_Header *header, Expression_Ast_Node* expr)
{
  this->expr = expr;
  this->header = header;
}

std::string Expression_Resolver::resolve(Tabular_Data_Row &data_row)
{
  if (this->expr->type == Ast_Node_Type::String_Literal_Expression_Ast_Node)
  {
    auto string_expr = static_cast<String_Literal_Expression_Ast_Node*>(this->expr);
    auto resolver = String_Literal_Resolver(string_expr->value);
    return resolver.resolve(data_row);
  }
  else if (this->expr->type == Ast_Node_Type::Number_Literal_Expression_Ast_Node)
  {
    auto number_expr = static_cast<Number_Literal_Expression_Ast_Node*>(this->expr);
    auto resolver = Number_Literal_Resolver(number_expr->value);
    return resolver.resolve(data_row);
  }
  else if (this->expr->type == Ast_Node_Type::Ident_Expression_Ast_Node)
  {
    auto ident_expr = static_cast<Ident_Expression_Ast_Node*>(this->expr);
    auto resolver = Field_By_Name_Resolver(*this->header, ident_expr->ident_name);
    return resolver.resolve(data_row);
  }
  else if (this->expr->type == Ast_Node_Type::Binary_Expression_Ast_Node)
  {
    auto bin_expr = static_cast<Binary_Expression_Ast_Node*>(this->expr);
    Expression_Resolver resolver_left = Expression_Resolver(this->header, bin_expr->left.get());
    Expression_Resolver resolver_right = Expression_Resolver(this->header, bin_expr->right.get());
    return resolver_left.resolve(data_row) + resolver_right.resolve(data_row);
  }
  else if (this->expr->type == Ast_Node_Type::Function_Call_Expression_Ast_Node)
  {
    auto call_expr = static_cast<Function_Call_Expression_Ast_Node*>(this->expr);
    auto resolver = Function_Call_Expression_Resolver(this->header, call_expr);
    return resolver.resolve(data_row);
  }
  else
  {
    // @note João, por hora não suporto outras expressões
    assert(false);
    return "";
  }
}

std::string Function_Call_Expression_Resolver::resolve(Tabular_Data_Row &data_row)
{
  if (this->call_expr->name == "CURRENT_DATE")
  {
    std::time_t current_date = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto local_time = std::localtime(&current_date);
    std::stringstream ss;
    ss << std::put_time(local_time, "%Y-%m-%d");

    return ss.str();
  }
  else if (this->call_expr->name == "LOWER")
  {
    auto expr = this->call_expr->argument_list.at(0);
    
    Expression_Resolver resolver = Expression_Resolver(this->header, expr);
    
    std::string value = resolver.resolve(data_row);
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
    return value;
  }
  else if (this->call_expr->name == "UPPER")
  {
    auto expr = this->call_expr->argument_list.at(0);
    
    Expression_Resolver resolver = Expression_Resolver(this->header, expr);
    
    std::string value = resolver.resolve(data_row);
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::toupper(c); });
    return value;
  }

  assert(false);
  return "[FUNCTION CALL RETURN]";
}

struct Binary_Expression_Resolver : Field_Resolver
{
  Binary_Expression_Ast_Node* bin_expr;
  Tabular_Data_Header *header;

  Binary_Expression_Resolver(Tabular_Data_Header *header, Binary_Expression_Ast_Node* bin_expr)
  {
    this->bin_expr = bin_expr;
    this->header = header;
  }

  std::string resolve([[maybe_unused]] Tabular_Data_Row &data_row)
  {
    Expression_Resolver resolver = Expression_Resolver(this->header, this->bin_expr);
    return resolver.resolve(data_row);
  }
};

bool known_function_name_and_argument_list(Function_Call_Expression_Ast_Node* call_expr)
{
  if (call_expr->name == "CURRENT_DATE")
  {
    return call_expr->argument_list.size() == 0;
  }
  else if (call_expr->name == "LOWER")
  {
    return call_expr->argument_list.size() == 1 && call_expr->argument_list.at(0)->inferred_type == Inferred_Type::String;
  }
  else if (call_expr->name == "UPPER")
  {
    return call_expr->argument_list.size() == 1 && call_expr->argument_list.at(0)->inferred_type == Inferred_Type::String;
  }
  else if (call_expr->name == "MAX")
  {
    // @wip @todo João, avaliar, deveria suportar texto também aparentemente...
    return call_expr->argument_list.size() == 1 && call_expr->argument_list.at(0)->inferred_type == Inferred_Type::Number;
  }

  return false;
}

Field_By_Name_Aggregation_Resolver::Field_By_Name_Aggregation_Resolver(Tabular_Data_Header &header, std::string field_name)
{
  auto it = std::find(header.begin(), header.end(), field_name);
  
  if (it == header.end())
  {
    assert(false);
    std::cout << "Error: field_name: " << field_name << " não existe no csv." << std::endl;
  }
  
  this->index_of_field = std::distance(header.begin(), it);
  this->field_name = field_name;
}

std::string Field_By_Name_Aggregation_Resolver::resolve(Tabular_Data_Row &grouped_data, [[maybe_unused]] vector<Tabular_Data_Row*> &rows)
{
  assert(this->index_of_field > -1);
  // @note João, não trata a exception in runtime? talvez, talvez fosse melhor retornar string vazia
  return grouped_data[this->index_of_field];
}
