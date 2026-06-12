#pragma once

#include <memory>
#include <string>
#include <vector>

#include "./ordered_map.hpp"
#include "./resolver.hpp"

// Dependências
#include "../lib/csv/src/csv.hpp"

enum class Aggregator_Type { Values, Subgrouping };


union Aggregated_Data;

struct Aggregator
{
  Aggregator_Type type;
  std::shared_ptr<Field_By_Name_Resolver> field_resolver;

  // para iterar
  int current_index;

  virtual ~Aggregator() = default;

  virtual void aggregate(CSV_Data_Row* row) = 0;

  /**
   * @brief define quantas cagetorias foram encontradas até o momento da chamada
   * 
   * @return size_t 
   */
  virtual size_t size() = 0;

  virtual std::unique_ptr<Aggregator> clone() = 0;

  virtual std::pair<std::string, Aggregated_Data> at(size_t index) = 0;

  virtual std::shared_ptr<Aggregator> get_subgrouping() = 0;

  /*
  std::unique_ptr<std::pair<std::vector<std::string>, CSV_Data_Row*>> get_next_group_value();
  */

  /**
   * @brief Retorna o número de agrupamentos definidos nesse agregador
   * 
   * @return size_t 
   */
  size_t grouping_depth();
};

struct Value_Aggregator: Aggregator
{
  /**
   * @brief Não é responsável pelos ponteiros
   */
  Ordered_Map<std::string, std::vector<CSV_Data_Row*>> ordered_data;
  
  Value_Aggregator(std::unique_ptr<Field_By_Name_Resolver> &field_resolver);

  void aggregate(CSV_Data_Row* row);

  size_t size();
  
  std::unique_ptr<Aggregator> clone();

  std::pair<std::string, Aggregated_Data> at(size_t index);

  std::shared_ptr<Aggregator> get_subgrouping();
};

struct Subgrouping_Aggregator: Aggregator
{
  /**
   * @brief Não é responsável pelos ponteiros
   */
  Ordered_Map<std::string, std::shared_ptr<Aggregator>>ordered_data;
  std::shared_ptr<Aggregator> subgrouping_aggregator;

  Subgrouping_Aggregator(std::unique_ptr<Field_By_Name_Resolver> &field_resolver, std::unique_ptr<Aggregator> &subgrouping_aggregator);

  void aggregate(CSV_Data_Row* row);

  size_t size();
  
  std::unique_ptr<Aggregator> clone();

  std::pair<std::string, Aggregated_Data> at(size_t index);

  std::shared_ptr<Aggregator> get_subgrouping();
};

union Aggregated_Data
{
  std::vector<CSV_Data_Row*>* list;
  Aggregator* aggregator;
};
