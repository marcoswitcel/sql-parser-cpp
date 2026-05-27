#pragma once

#include <assert.h>
#include <utility>
#include <string>
#include <vector>
#include <unordered_map>


template<typename Key_Type, typename Value_Type>
struct Ordered_Map
{
  std::vector<std::pair<Key_Type, Value_Type>> ordered_list;
  std::unordered_map<Key_Type, size_t> map;

  Value_Type* lookup(const Key_Type &key)
  {
    auto itetator = this->map.find(key);

    if (itetator == map.end())
    {
      return NULL;
    }
    
    return &(this->ordered_list.at(itetator->second).second);
  }

  bool put(const Key_Type &key, Value_Type value)
  {
    auto iterator = this->map.find(key);

    if (iterator != this->map.end())
    {
      this->ordered_list[iterator->second].second = value;
      return false;
    }

    this->ordered_list.push_back(std::make_pair(key, value));
    this->map[key] = this->ordered_list.size() - 1;

    return true;
  }

  size_t size()
  {
    assert(this->ordered_list.size() == this->map.size());

    return this->ordered_list.size();
  }

  // @todo João, implementar métodos "remove" e "clear"
  // @todo João, melhorar iteração, ocultar a lista...

};
