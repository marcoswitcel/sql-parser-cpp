#pragma once

#include <string>
#include <assert.h>
#include <cstdint>
#include <cctype>

#include "./sql-parse-context.hpp"


std::string get_description(Token_Type &token_type)
{
  switch (token_type)
  {
    case NONE: return "NONE";
    case SELECT: return "SELECT";
    case FROM: return "FROM";
    case ASTERISK: return "ASTERISK";
    case IDENT: return "IDENT";
  }

  assert(false);

  // @note Talvez retornar o valor junto? castear para number e printar
  return "[Invalid Token_Type]";
}

std::string Token::to_string()
{
  return "Token@{ .type = " + get_description(this->type) + " }";
}


SQL_Parse_Context::SQL_Parse_Context(std::string source)
{
  this->source = source;
  this->index = 0;
}

int32_t SQL_Parse_Context::peek_char()
{
  if (this->is_finished()) return END_OF_SOURCE;

  return static_cast<int32_t>(this->source[this->index]);
}

int32_t SQL_Parse_Context::peek_n_char(size_t n)
{
  size_t index_with_n = this->index + n;

  if (index_with_n >= this->source.length()) return END_OF_SOURCE;

  return static_cast<int32_t>(this->source[index_with_n]);
}



int32_t SQL_Parse_Context::eat_char()
{
  if (this->is_finished()) return END_OF_SOURCE;

  return static_cast<int32_t>(this->source[this->index++]);
}

inline bool SQL_Parse_Context::is_finished()
{
  return this->index >= this->source.length();
}

inline bool SQL_Parse_Context::is_whitespace(char value)
{
  return (value == ' ' || value == '\t' || value == '\r' || value == '\n');
}


void SQL_Parse_Context::skip_whitespace()
{
  
  while (this->is_finished())
  {
    int32_t value = this->peek_char();
    if (value < 0) return;

    if (!this->is_whitespace(static_cast<char>(value))) return;
  }
}

Parse_Function terminals[] = {
  try_parse_select
};

constexpr size_t terminals_length = sizeof(terminals[0]) / sizeof(terminals) ;


Token SQL_Parse_Context::eat_token()
{
  Token token = { .type = NONE };

  this->skip_whitespace();
  
  for (size_t i = 0; i < terminals_length; i++)
  {
    Parse_Function func = terminals[i];
    bool success = false;

    func(this, &token, &success);
  
    if (success)
    {
      return token;
    }
  }

  this->error = true;
  return token;
}


void try_parse_select(SQL_Parse_Context* parser, Token *token, bool *success)
{
  std::string token_identifier = "select";
  const auto index_after_last_char = token_identifier.size();
  
  bool equal = true;
  for (size_t i = 0; i < token_identifier.size(); i++)
  {
    char c = std::tolower(parser->peek_n_char(i));
    if (c != token_identifier.at(i))
    {
      equal = false;
      break;
    }
  }

  if (equal &&
    parser->peek_n_char(index_after_last_char) != END_OF_SOURCE &&
    parser->is_whitespace(parser->peek_n_char(index_after_last_char)))
  {
    for (size_t i = 0; i < token_identifier.size(); i++)
    {
      parser->eat_char();
    }
    parser->eat_char();

    token->type = Token_Type::SELECT;
    *success = true;
    return;
  }
  
  token->type = Token_Type::NONE;
  *success = false;
}