#pragma once

#include <vector>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>

#include "./trace.hpp"

/**
 * @brief Separa a string provida em várias partes (explode) usando o delimitador
 * como critério de separação
 * 
 * @param value 
 * @param delimiter 
 * @return std::vector<std::string> 
 */
std::vector<std::string> split_by(std::string value, char delimiter)
{
  std::vector<std::string> list;

  size_t scanner_index = 0;
  size_t current_word_start_index = 0;
  
  for (; scanner_index < value.length(); scanner_index++)
  {
    if (value.at(scanner_index) == delimiter)
    {
      list.push_back(value.substr(current_word_start_index, scanner_index - current_word_start_index));
      current_word_start_index = scanner_index + 1;
    }
  }

  list.push_back(value.substr(current_word_start_index, scanner_index - current_word_start_index));

  return list;
}

bool contains(std::string text, char needle)
{
  for (size_t i = 0; i < text.length(); i++)
  {
    if (text.at(i) == needle)
    {
      return true;
    }
  }

  return false;
}

bool contains(std::vector<std::string> &list, std::string &neddle_value)
{
  return std::find(list.begin(), list.end(), neddle_value) != list.end();
}

int64_t index_of(std::vector<std::string> &list, std::string &neddle_value)
{
  for (size_t i = 0; i < list.size(); i++)
  {
    if (list.at(i) == neddle_value)
    {
      return i;
    }
  }

  return -1;
}

