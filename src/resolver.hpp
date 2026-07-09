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
  Tabular_Data_Header* header_grouped_data;
  Tabular_Data_Header* header_data;
  

  Function_Call_Expression_Aggregation_Resolver(Tabular_Data_Header* header_grouped_data,Tabular_Data_Header* header_data, Function_Call_Expression_Ast_Node* call_expr)
  {
    this->call_expr = call_expr;
    this->header_grouped_data = header_grouped_data;
    this->header_data = header_data;
  }

  std::string resolve(Tabular_Data_Row &grouped_data, vector<Tabular_Data_Row*> &rows);
};

struct Builtin_Function_Definition
{
  std::string name;
  bool is_aggregation_needed;
};

/**
 * @brief Existe apenas para listar as funções e expectativas 
 * @note Ideias de funções
 * - MEDIA (agregação) soma e divide pelo total
 * - TRUNCATE (padrão) trunca uma string
 * - FIRST (agregação) retorna a primeira ocorrência 
 */
static Builtin_Function_Definition functions_builtin[] = {
  // Funções
  // Retorna a data atual
  { .name = "CURRENT_DATE", .is_aggregation_needed = false, },
  // Converte a string para lowercase
  { .name = "LOWER", .is_aggregation_needed = false, },
  // Converte a string para uppercase
  { .name = "UPPER", .is_aggregation_needed = false, },
  // Corta uma string
  { .name = "SUBSTRING", .is_aggregation_needed = false, },
  
  // Funções de agregação
  // Retorna o maior valor numérico do grupo para a dada coluna
  { .name = "MAX", .is_aggregation_needed = true, },
  // Retorna o menor valor numérico do grupo para a dada coluna
  { .name = "MIN", .is_aggregation_needed = true, },
  // Retorna a quantidade de registros do grupo para a dada coluna
  { .name = "COUNT", .is_aggregation_needed = true, },
  // Retorna a soma dos valores do grupo para a dada coluna
  { .name = "SUM", .is_aggregation_needed = true, },
  // Retorna a média dos valores do grupo para a dada coluna
  { .name = "AVG", .is_aggregation_needed = true, },
  // Retorna o valor da primeira linha encontrada para a data coluna
  { .name = "FIRST_VALUE", .is_aggregation_needed = true, },
};
