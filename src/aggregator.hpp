#pragma once

#include <memory>
#include <string>
#include <vector>

#include "./ordered_map.hpp"
#include "./resolver.hpp"

// Dependências
#include "../lib/csv/src/csv.hpp"

enum class Aggregator_Type { Values, Subgrouping };

struct Aggregator
{
  Aggregator_Type type;
  std::shared_ptr<Field_By_Name_Resolver> field_resolver;

  virtual ~Aggregator() = default;

  virtual void aggregate(CSV_Data_Row* row) = 0;

  virtual size_t size() = 0;

  virtual std::unique_ptr<Aggregator> clone() = 0;
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
};
