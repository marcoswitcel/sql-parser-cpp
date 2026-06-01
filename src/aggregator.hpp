#pragma once

#include <memory>
#include <string>
#include <vector>

#include "./ordered_map.hpp"

// Dependências
#include "../lib/csv/src/csv.hpp"

enum class Aggregator_Type { Values, Subgrouping };

struct Aggregator
{
  Aggregator_Type type = Aggregator_Type::Values;

  std::unique_ptr<Aggregator> next_aggregator;
  std::unique_ptr<Ordered_Map<std::string, std::vector<CSV_Data_Row*>>> data;

  ~Aggregator() = default;

  // métodos de setup
  void add_subgrouping(std::unique_ptr<Aggregator> aggregator);

  void aggregate(CSV_Data_Row* row);
};
