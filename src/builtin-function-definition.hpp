#pragma once

#include <string>
#include <string_view>

struct Builtin_Function_Definition
{
  std::string name;
  bool is_aggregation_needed;
};

// @todo João, trocar o nome das funções por instâncias do enum para facilitar as comparações
enum class Builtin_Function_Names
{
  UNKNOWN,
  // nomes válidos
  CURRENT_DATE,
  LOWER,
  UPPER,
  SUBSTRING,
  COALESCE,
  TO_NUMBER,
  MAX,
  MIN,
  COUNT,
  SUM,
  AVG,
  FIRST_VALUE,
};

constexpr std::string_view to_string(Builtin_Function_Names name)
{
  switch (name)
  {
    case Builtin_Function_Names::CURRENT_DATE: return "CURRENT_DATE";
    case Builtin_Function_Names::LOWER: return "LOWER";
    case Builtin_Function_Names::UPPER: return "UPPER";
    case Builtin_Function_Names::SUBSTRING: return "SUBSTRING";
    case Builtin_Function_Names::COALESCE: return "COALESCE";
    case Builtin_Function_Names::TO_NUMBER: return "TO_NUMBER";
    case Builtin_Function_Names::MAX: return "MAX";
    case Builtin_Function_Names::MIN: return "MIN";
    case Builtin_Function_Names::COUNT: return "COUNT";
    case Builtin_Function_Names::SUM: return "SUM";
    case Builtin_Function_Names::AVG: return "AVG";
    case Builtin_Function_Names::FIRST_VALUE: return "FIRST_VALUE";
  }

  return "[UNKNOWN]";
}

inline Builtin_Function_Names lookup(const std::string &name)
{
  if (name == "CURRENT_DATE") return Builtin_Function_Names::CURRENT_DATE;
  else if (name == "LOWER") return Builtin_Function_Names::LOWER;
  else if (name == "UPPER") return Builtin_Function_Names::UPPER;
  else if (name == "SUBSTRING") return Builtin_Function_Names::SUBSTRING;
  else if (name == "COALESCE") return Builtin_Function_Names::COALESCE;
  else if (name == "TO_NUMBER") return Builtin_Function_Names::TO_NUMBER;
  else if (name == "MAX") return Builtin_Function_Names::MAX;
  else if (name == "MIN") return Builtin_Function_Names::MIN;
  else if (name == "COUNT") return Builtin_Function_Names::COUNT;
  else if (name == "SUM") return Builtin_Function_Names::SUM;
  else if (name == "AVG") return Builtin_Function_Names::AVG;
  else if (name == "FIRST_VALUE") return Builtin_Function_Names::FIRST_VALUE;
 
  return Builtin_Function_Names::UNKNOWN;
}

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
  // Retorna primeiro valor não "vazio"
  { .name = "COALESCE", .is_aggregation_needed = false, },
  // Recebe uma expressão e retonar o valor numérico que ela representa,
  // ou NaN em caso de erros de parsing ou valor vazio. Se possuir um valor default
  // retorna o default
  { .name = "TO_NUMBER", .is_aggregation_needed = false, },
  
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

/**
 * @brief identifica onde começa a sessão de funções de aggregação dentro do array `function_builtin`
 * 
 */
constexpr size_t start_index_of_aggregation_functions = 6;
constexpr size_t functions_builtin_length = sizeof(functions_builtin) / sizeof(functions_builtin[0]);
