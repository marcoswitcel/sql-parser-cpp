#pragma once

#include <memory>

#include "./aggregator.hpp"


size_t Aggregator::grouping_depth()
{
  size_t count = 1;

  std::shared_ptr<Aggregator> group = this->get_subgrouping();

  while (group)
  {
    count++;
    group = group->get_subgrouping();
  }

  return count;
}

std::unique_ptr<Tabular_Data_Header> Aggregator::get_header()
{
  auto header = std::make_unique<Tabular_Data_Header>();
  header->push_back(this->field_resolver->field_name);

  std::shared_ptr<Aggregator> group = this->get_subgrouping();
  while (group)
  {
    header->push_back(group->field_resolver->field_name);
    group = group->get_subgrouping();
  }

  if (header->empty()) return {};

  return header;
}

std::unique_ptr<Group_Value> Aggregator::get_next_group_value()
{
  // @note João, pra deixar essa função mais rápida seria necessário preservar a stack e os "field names"
  // mas não quero fazer isso agora...
  std::vector<Aggregator*> stack;
  stack.push_back(this);
  
  std::vector<std::string> field_names;

  while (stack.size() > 0)
  {
    auto aggregator = stack.back();
    
    // inicializa o iterador caso não tenha iniciado
    if (aggregator->current_index < 0) aggregator->current_index = 0;

    if (aggregator->type == Aggregator_Type::Values)
    {
      auto value_aggregator = static_cast<Value_Aggregator*>(aggregator);

      if (value_aggregator->current_index < static_cast<int>(value_aggregator->size()))
      {
        auto value = value_aggregator->at(value_aggregator->current_index);
        value_aggregator->current_index++;
        field_names.push_back(value.first);

        return std::make_unique<Group_Value>(field_names, *value.second.list);
      }
    }
    else if (aggregator->type == Aggregator_Type::Subgrouping)
    {
      auto subgrouping_aggregator = static_cast<Subgrouping_Aggregator*>(aggregator);

      if (subgrouping_aggregator->current_index < static_cast<int>(subgrouping_aggregator->size()))
      {
        auto value = subgrouping_aggregator->at(subgrouping_aggregator->current_index);
        field_names.push_back(value.first);
        
        stack.push_back(value.second.aggregator);
        continue;
      }
    }

    stack.pop_back();

    if (stack.size() > 0)
    {
      auto next = stack.back();
      next->current_index++;
    }
    
    if (field_names.size() > 0) field_names.pop_back();
  }
  
  return {};
}


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

std::pair<std::string, Aggregated_Data> Value_Aggregator::at(size_t index)
{
  auto &pair = this->ordered_data.ordered_list.at(index);
  return std::make_pair(pair.first, Aggregated_Data { .list =  &pair.second });
}

std::shared_ptr<Aggregator> Value_Aggregator::get_subgrouping()
{
  return {}; // empty shared pointer
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

std::pair<std::string, Aggregated_Data> Subgrouping_Aggregator::at(size_t index)
{
  auto &pair = this->ordered_data.ordered_list.at(index);
  return std::make_pair(pair.first, Aggregated_Data { .aggregator = pair.second.get() });
}

std::shared_ptr<Aggregator> Subgrouping_Aggregator::get_subgrouping()
{
  return this->subgrouping_aggregator;
}
