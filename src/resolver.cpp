#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <limits>
#include <cmath>

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
  else if (this->call_expr->name == "SUBSTRING")
  {
    auto arg0_text = this->call_expr->argument_list.at(0);
    auto arg1_start = this->call_expr->argument_list.at(1);
    
    Expression_Resolver resolver_arg0 = Expression_Resolver(this->header, arg0_text);
    Expression_Resolver resolver_arg1 = Expression_Resolver(this->header, arg1_start);
    
    std::string text_value = resolver_arg0.resolve(data_row);
    std::string start_value = resolver_arg1.resolve(data_row);
    size_t start, end;

    try 
    {
      start = std::stoi(start_value);

      if (this->call_expr->argument_list.size() == 3)
      {
        auto arg2_end = this->call_expr->argument_list.at(2);
        Expression_Resolver resolver_arg2 = Expression_Resolver(this->header, arg2_end);
        std::string end_value = resolver_arg2.resolve(data_row);
        
        end = std::stoi(end_value);
      }
      else
      {
        end = text_value.size();
      }

      if (start > text_value.size())
      {
        start = text_value.size();
      }

      return text_value.substr(start, end);
    }
    catch (std::invalid_argument& ex)
    {
      // @note João, não deve aconceter
      assert(false);
    }
    catch (std::out_of_range& ex)
    {
      // @note João, pode acontecer, avaliar o que fazer...
      assert(false);
    }
    
    return "[FUNCTION CALL RETURN]";
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
  else if (call_expr->name == "SUBSTRING")
  {
    if (call_expr->argument_list.size() == 3)
    {
      return call_expr->argument_list.at(0)->inferred_type == Inferred_Type::String
        && call_expr->argument_list.at(1)->inferred_type == Inferred_Type::Number
        && call_expr->argument_list.at(2)->inferred_type == Inferred_Type::Number;
    }
    if (call_expr->argument_list.size() == 2)
    {
      return call_expr->argument_list.at(0)->inferred_type == Inferred_Type::String
        && call_expr->argument_list.at(1)->inferred_type == Inferred_Type::Number;
    }

    return false;
  }
  else if (call_expr->name == "MAX")
  {
    return call_expr->argument_list.size() == 1 && call_expr->argument_list.at(0)->type == Ast_Node_Type::Ident_Expression_Ast_Node;
  }
  else if (call_expr->name == "MIN")
  {
    return call_expr->argument_list.size() == 1 && call_expr->argument_list.at(0)->type == Ast_Node_Type::Ident_Expression_Ast_Node;
  }
  else if (call_expr->name == "COUNT")
  {
    if (call_expr->argument_list.size() != 1) return false;
    
    auto expr = call_expr->argument_list.at(0);

    if (auto ident = Cast_If(Ident_Expression_Ast_Node, *expr))
    {
      if (ident->ident_name == "*") return true;
    }
  }
  else if (call_expr->name == "SUM")
  {
    return call_expr->argument_list.size() == 1 && call_expr->argument_list.at(0)->type == Ast_Node_Type::Ident_Expression_Ast_Node;
  }
  else if (call_expr->name == "AVG")
  {
    return call_expr->argument_list.size() == 1 && call_expr->argument_list.at(0)->type == Ast_Node_Type::Ident_Expression_Ast_Node;
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

std::string Function_Call_Expression_Aggregation_Resolver::resolve([[maybe_unused]] Tabular_Data_Row &grouped_data, vector<Tabular_Data_Row*> &rows)
{
  if (this->call_expr->name == "COUNT")
  {
    return std::to_string(rows.size());
  }
  else if (this->call_expr->name == "MAX")
  {
    auto expr = this->call_expr->argument_list.at(0);
    
    Expression_Resolver resolver = Expression_Resolver(this->header_data, expr);
    
    // @note Não é possível ter 0 linhas, porém, é possível que todas as linhas falhem
    // no processo de parse, por isso usamos NaN
    double max_value = std::numeric_limits<double>::quiet_NaN();

    for (auto data_row : rows)
    {
      auto raw_value = resolver.resolve(*data_row);

      try 
      {
        auto value = std::stod(raw_value);

        if (value > max_value || std::isnan(max_value))
        {
          max_value = value;
        }
      }
      catch (std::invalid_argument& ex) {}
      catch (std::out_of_range& ex) {}
    }

    return std::to_string(max_value);
  }
  else if (this->call_expr->name == "MIN")
  {
    auto expr = this->call_expr->argument_list.at(0);
    
    Expression_Resolver resolver = Expression_Resolver(this->header_data, expr);
    
    // @note Não é possível ter 0 linhas, porém, é possível que todas as linhas falhem
    // no processo de parse, por isso usamos NaN
    double max_value = std::numeric_limits<double>::quiet_NaN();

    for (auto data_row : rows)
    {
      auto raw_value = resolver.resolve(*data_row);

      try 
      {
        auto value = std::stod(raw_value);

        if (value < max_value || std::isnan(max_value))
        {
          max_value = value;
        }
      }
      catch (std::invalid_argument& ex) {}
      catch (std::out_of_range& ex) {}
    }

    return std::to_string(max_value);
  }
  else if (this->call_expr->name == "SUM")
  {
    auto expr = this->call_expr->argument_list.at(0);
    
    Expression_Resolver resolver = Expression_Resolver(this->header_data, expr);
    
    // @note Não é possível ter 0 linhas, porém, é possível que todas as linhas falhem
    // no processo de parse, por isso usamos NaN
    double sum_value = std::numeric_limits<double>::quiet_NaN();

    for (auto data_row : rows)
    {
      auto raw_value = resolver.resolve(*data_row);

      try 
      {
        auto value = std::stod(raw_value);

        if (std::isnan(sum_value))
        {
          sum_value = value;
        }
        else
        {
          sum_value += value;
        }
      }
      catch (std::invalid_argument& ex) {}
      catch (std::out_of_range& ex) {}
    }

    return std::to_string(sum_value);
  }
  else if (this->call_expr->name == "AVG")
  {
    auto expr = this->call_expr->argument_list.at(0);
    
    Expression_Resolver resolver = Expression_Resolver(this->header_data, expr);
    
    // @note Não é possível ter 0 linhas, porém, é possível que todas as linhas falhem
    // no processo de parse, por isso usamos NaN
    double sum_value = std::numeric_limits<double>::quiet_NaN();

    for (auto data_row : rows)
    {
      auto raw_value = resolver.resolve(*data_row);

      try 
      {
        auto value = std::stod(raw_value);

        if (std::isnan(sum_value))
        {
          sum_value = value;
        }
        else
        {
          // @todo João, deveria somar NaNs?
          sum_value += value;
        }
      }
      catch (std::invalid_argument& ex) {}
      catch (std::out_of_range& ex) {}
    }

    return std::to_string(sum_value / rows.size());
  }


  assert(false);
  return "[AGGREGATION FUNCTION CALL RETURN]";
}
