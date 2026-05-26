#pragma once

#include <utility>
#include <string>
#include <vector>
#include <unordered_map>


template<typename Key_Type, typename Value_Type>
class Ordered_Map
{
  std::vector<std::pair<Key_Type, Value_Type>> ordered_list;
  std::unordered_map<Key_Type, size_t> map;

  Value_Type* find(const Key_Type &key)
  {
    auto itetator = this->map.find(key);

    if (itetator == map.end())
    {
      return NULL;
    }
    
    return &(this->ordered_list.at(itetator->second).second);
  }

};
