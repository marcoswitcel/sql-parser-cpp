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
  std::unique_ptr<Field_By_Name_Resolver> field_resolver;

  ~Aggregator() = default;

  virtual void aggregate(CSV_Data_Row* row) = 0;

  virtual std::unique_ptr<Aggregator> clone() = 0;
};

struct Value_Aggregator: Aggregator
{
  Aggregator_Type type = Aggregator_Type::Values;
  /**
   * @brief Não é responsável pelos ponteiros
   */
  Ordered_Map<std::string, std::vector<CSV_Data_Row*>> ordered_data;
  

  Value_Aggregator(std::unique_ptr<Field_By_Name_Resolver> &field_resolver)
  {
    this->field_resolver = std::move(field_resolver);
  };

  void aggregate(CSV_Data_Row* row);
  
  std::unique_ptr<Aggregator> clone();
};

struct Subgrouping_Aggregator: Aggregator
{
  Aggregator_Type type = Aggregator_Type::Subgrouping;
  /**
   * @brief Não é responsável pelos ponteiros
   */
  Ordered_Map<std::string, Aggregator> ordered_data;

  Subgrouping_Aggregator(std::unique_ptr<Field_By_Name_Resolver> &field_resolver)
  {
    this->field_resolver = std::move(field_resolver);
  };

  void aggregate(CSV_Data_Row* row);
  
  std::unique_ptr<Aggregator> clone();
};
