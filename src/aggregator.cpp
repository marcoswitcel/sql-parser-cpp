#pragma once

#include <memory>

#include "./aggregator.hpp"


void Value_Aggregator::aggregate(CSV_Data_Row* row)
{
  auto result = this->field_resolver->resolve(*row);
        
  auto result_set = this->ordered_data.lookup(result);

  // se tem adiciona
  if (result_set)
  {
    result_set->push_back(row);
  }
  else
  {
    // se não tem cria e adiciona e vincula
    std::vector<CSV_Data_Row*> new_set;
    new_set.push_back(row);

    this->ordered_data.put(result, new_set);
  }
}

std::unique_ptr<Aggregator> Value_Aggregator::clone()
{
  return std::make_unique<Value_Aggregator>(*this);
}

void Subgrouping_Aggregator::aggregate(CSV_Data_Row* row)
{
  auto result = this->field_resolver->resolve(*row);
        
  auto result_set = this->ordered_data.lookup(result);

  // se tem adiciona
  if (result_set)
  {
    result_set->aggregate(row);
  }
  else
  {
    // @todo João, aqui precisa ter uma factorie?
    // se não tem cria e adiciona e vincula
    // std::vector<CSV_Data_Row*> new_set;
    // new_set.push_back(row);

    //this->ordered_data.put(result, new_set);
  }
}

std::unique_ptr<Aggregator> Subgrouping_Aggregator::clone()
{
  return std::make_unique<Subgrouping_Aggregator>(*this);
}