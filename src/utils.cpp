#pragma once

#include <vector>
#include <iostream>
#include <sstream>
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
