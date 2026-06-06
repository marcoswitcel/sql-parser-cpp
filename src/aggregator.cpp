#pragma once

#include <memory>

#include "./aggregator.hpp"


Value_Aggregator::Value_Aggregator(std::unique_ptr<Field_By_Name_Resolver> &field_resolver)
{
  this->type = Aggregator_Type::Values;

  this->field_resolver = std::move(field_resolver);
}

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

size_t Value_Aggregator::size()
{
  return this->ordered_data.size();
}

std::unique_ptr<Aggregator> Value_Aggregator::clone()
{
  return std::make_unique<Value_Aggregator>(*this);
}

Aggregated_Data Value_Aggregator::at(size_t index)
{
  auto &pair = this->ordered_data.ordered_list.at(index);
  return { .list =  &pair.second };
}

Subgrouping_Aggregator::Subgrouping_Aggregator(std::unique_ptr<Field_By_Name_Resolver> &field_resolver, std::unique_ptr<Aggregator> &subgrouping_aggregator)
{
  this->type = Aggregator_Type::Subgrouping;

  this->subgrouping_aggregator = std::move(subgrouping_aggregator);
  this->field_resolver = std::move(field_resolver);
}

void Subgrouping_Aggregator::aggregate(CSV_Data_Row* row)
{
  auto result = this->field_resolver->resolve(*row);
        
  auto result_set = this->ordered_data.lookup(result);

  // se tem adiciona
  if (result_set)
  {
    (*result_set)->aggregate(row);
  }
  else
  {
    // se não tem cria, agrega e vincula
    auto new_aggregator = this->subgrouping_aggregator->clone();
    new_aggregator->aggregate(row);

    this->ordered_data.put(result, std::move(new_aggregator));
  }
}

size_t Subgrouping_Aggregator::size()
{
  return this->ordered_data.size();
}

std::unique_ptr<Aggregator> Subgrouping_Aggregator::clone()
{
  return std::make_unique<Subgrouping_Aggregator>(*this);
}

Aggregated_Data Subgrouping_Aggregator::at(size_t index)
{
  auto &pair = this->ordered_data.ordered_list.at(index);
  return { .aggregator = pair.second.get() };
}