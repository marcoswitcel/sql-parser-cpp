#pragma once

#include <string>
#include <vector>

#include "./ast_node.hpp"

#include "../lib/csv/src/csv.hpp"

struct Field_Resolver
{
  virtual ~Field_Resolver() = default;

  /**
   * @brief resolve o valor de uma expressão de acordo com a 'linha' contendo os pertinentes ao registro
   * @note João, não foi definido uma 'interface' para retornar erros no processo de 'resolver' os valores, apenas
   * asserts existem no momento.
   * 
   * @param data_row 
   * @return std::string 
   */
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

// @note provavelmente deveria chamar esse resolver de 'Ident_Resolver', se fosse pra ser um 'field resolver' provavelmente
// deveria operar em cima de todas expressões válidas para 'fields' do select
// Mas um ponto que percebi depois de escrever o comentário acima é que no contexto dos "Resolver's", nem sempre um ident refere
// a uma coluna da tabela, então talvez Field_By_Name seja mais claro mesmo. Até porque os "resolver's" eram apenas para uso
// na resolução de  valores para colunas, agora estão sendo usados na cláusula "where" também.
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

struct Expression_Resolver : Field_Resolver
{
  Expression_Ast_Node* expr;
  CSVData *csv;

  Expression_Resolver(CSVData *csv, Expression_Ast_Node* expr);
  std::string resolve(std::vector<std::string> &data_row);
};
