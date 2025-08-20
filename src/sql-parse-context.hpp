#pragma once

#include <string>
#include <cstdint>


constexpr int32_t END_OF_SOURCE = -9999;

struct SQL_Parse_Context
{
  std::string source;
  uint64_t index = 0;

  SQL_Parse_Context(std::string source)
  {
    this->source = source;
    this->index = 0;
  }

  int32_t peek_char()
  {
    if (this->is_finished()) return END_OF_SOURCE;

    return static_cast<int32_t>(this->source[this->index]);
  }

  int32_t eat_char()
  {
    if (this->is_finished()) return END_OF_SOURCE;

    return static_cast<int32_t>(this->source[this->index++]);
  }

  inline bool is_finished()
  {
    return this->index >= this->source.length();
  }
};
