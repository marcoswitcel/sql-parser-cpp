#pragma once

#include <string>
#include <vector>

#include "./ast_node.hpp"


using std::vector; 
using Tabular_Data_Row = std::vector<std::string>;
using Tabular_Data_Header = Tabular_Data_Row;


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
  virtual std::string resolve(Tabular_Data_Row &data_row) = 0;
};

struct Function_Call_Expression_Resolver : Field_Resolver
{
  Function_Call_Expression_Ast_Node* call_expr;
  Tabular_Data_Header *header;

  Function_Call_Expression_Resolver(Tabular_Data_Header *header, Function_Call_Expression_Ast_Node* call_expr)
  {
    this->call_expr = call_expr;
    this->header = header;
  }

  std::string resolve(Tabular_Data_Row &data_row);
};

// @note provavelmente deveria chamar esse resolver de 'Ident_Resolver', se fosse pra ser um 'field resolver' provavelmente
// deveria operar em cima de todas expressões válidas para 'fields' do select
// Mas um ponto que percebi depois de escrever o comentário acima é que no contexto dos "Resolver's", nem sempre um ident refere
// a uma coluna da tabela, então talvez Field_By_Name seja mais claro mesmo. Até porque os "resolver's" eram apenas para uso
// na resolução de  valores para colunas, agora estão sendo usados na cláusula "where" também.
struct Field_By_Name_Resolver : Field_Resolver
{
  int64_t index_of_field = -1;
  std::string field_name;

  Field_By_Name_Resolver(Tabular_Data_Header &header, std::string field_name);

  std::string resolve(Tabular_Data_Row &data_row);
};

struct String_Literal_Resolver : Field_Resolver
{
  std::string value;

  String_Literal_Resolver(std::string string_value)
  {
    this->value = string_value;
  }

  std::string resolve([[maybe_unused]] Tabular_Data_Row &data_row);
};

struct Number_Literal_Resolver : Field_Resolver
{
  int64_t value;

  Number_Literal_Resolver(int64_t value)
  {
    this->value = value;
  }

  std::string resolve([[maybe_unused]] Tabular_Data_Row &data_row);
};

struct Expression_Resolver : Field_Resolver
{
  Expression_Ast_Node* expr;
  Tabular_Data_Header *header;

  Expression_Resolver(Tabular_Data_Header *header, Expression_Ast_Node* expr);
  
  std::string resolve(Tabular_Data_Row &data_row);
};

struct Aggregation_Field_Resolver
{
  virtual ~Aggregation_Field_Resolver() = default;

  virtual std::string resolve(Tabular_Data_Row &grouped_data, vector<Tabular_Data_Row*> &rows) = 0;
};

struct Field_By_Name_Aggregation_Resolver : Aggregation_Field_Resolver
{
  int64_t index_of_field = -1;
  std::string field_name;

  Field_By_Name_Aggregation_Resolver(Tabular_Data_Header &header, std::string field_name);

  std::string resolve(Tabular_Data_Row &grouped_data, vector<Tabular_Data_Row*> &rows);
};

struct Function_Call_Expression_Aggregation_Resolver : Aggregation_Field_Resolver
{
  Function_Call_Expression_Ast_Node* call_expr;
  Tabular_Data_Header *header;

  Function_Call_Expression_Aggregation_Resolver(Tabular_Data_Header *header, Function_Call_Expression_Ast_Node *call_expr)
  {
    this->call_expr = call_expr;
    this->header = header;
  }

  std::string resolve(Tabular_Data_Row &grouped_data, vector<Tabular_Data_Row*> &rows);
};

// @todo João, precisa retornar uma mensagem clara no caso de current_date(1) ou currante_date('1'), aí sim podemos
// continuar com o COUNT(*)

