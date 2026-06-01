#pragma once

#include <memory>

#include "./aggregator.hpp"


void Aggregator::add_subgrouping(std::unique_ptr<Aggregator> aggregator)
{
  this->type = Aggregator_Type::Subgrouping;
  this->next_aggregator = std::move(aggregator);
};

void Aggregator::aggregate(CSV_Data_Row* row)
{
  if (this->type == Aggregator_Type::Subgrouping)
  {
    this->next_aggregator->aggregate(row);
    return;
  }
}
