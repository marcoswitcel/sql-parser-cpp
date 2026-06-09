#pragma once

#include <memory>
#include <string>
#include <vector>

#include "./aggregator.hpp"

// Dependências
#include "../lib/csv/src/csv.hpp"

struct Group_Extractor
{
  std::unique_ptr<Aggregator> aggregator;

  Group_Extractor(std::unique_ptr<Aggregator> &aggregator);

  std::unique_ptr<std::pair<std::vector<std::string>, CSV_Data_Row>> extract_a_group();
};
