#pragma once

#include <string>
#include <assert.h>
#include <cstdint>

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


Token SQL_Parse_Context::eat_token()
{
  this->skip_whitespace();

  bool success = false;
  Token token = { .type = NONE };

  try_parse_select(this, &token, &success);

  return token;
}


void try_parse_select(SQL_Parse_Context* parser, Token *token, bool *success)
{
  int32_t c = parser->peek_char();


  token->type = Token_Type::SELECT;

  *success = false;
}