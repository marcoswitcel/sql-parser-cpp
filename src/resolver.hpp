#pragma once

#include <string>
#include <vector>

#include "./ast_node.hpp"

#include "../lib/csv/src/csv.hpp"

struct Field_Resolver
{
  virtual ~Field_Resolver() = default;

  virtual std::string resolve(std::vector<std::string> &data_row) = 0;
};

struct Function_Call_Expression_Resolver : Field_Resolver
{
  Function_Call_Expression_Ast_Node* call_expr;
  CSVData *csv;

  Function_Call_Expression_Resolver(CSVData *csv, Function_Call_Expression_Ast_Node* call_expr)
  {
    this->call_expr = call_expr;
    this->csv = csv;
  }

  std::string resolve(std::vector<std::string> &data_row);
};

struct Field_By_Name_Resolver : Field_Resolver
{
  int64_t index_of_field = -1;

  Field_By_Name_Resolver(CSVData &csv, std::string field_name);

  std::string resolve(std::vector<std::string> &data_row);
};

struct String_Literal_Resolver : Field_Resolver
{
  std::string value;

  String_Literal_Resolver(std::string string_value)
  {
    this->value = string_value;
  }

  std::string resolve([[maybe_unused]] std::vector<std::string> &data_row);
};

struct Number_Literal_Resolver : Field_Resolver
{
  int64_t value;

  Number_Literal_Resolver(int64_t value)
  {
    this->value = value;
  }

  std::string resolve([[maybe_unused]] std::vector<std::string> &data_row);
};
