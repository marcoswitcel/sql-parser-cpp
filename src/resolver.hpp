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
