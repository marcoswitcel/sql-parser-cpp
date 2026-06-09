#pragma once

#include <memory>

#include "./group_extractor.hpp"

Group_Extractor::Group_Extractor(std::unique_ptr<Aggregator> &aggregator)
{
  this->aggregator = std::move(aggregator);
}

std::unique_ptr<std::pair<std::vector<std::string>, CSV_Data_Row>> Group_Extractor::extract_a_group()
{
  // @todo João, implementar
}
