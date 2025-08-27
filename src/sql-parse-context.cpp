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
    case COMMA: return "COMMA";
    case IDENT: return "IDENT";
  }

  assert(false);

  // @note Talvez retornar o valor junto? castear para number e printar
  return "[Invalid Token_Type]";
}

  
std::string Ident_Token::to_string()
{
  return "Ident_Token { .ident = '" + this->ident + "' }";
}

std::string Token::to_string()
{
  std::string desc = "Token { .type = " + get_description(this->type) + ", .data = ";

  if (this->data && this->type == Token_Type::IDENT)
  {
    desc += static_cast<Ident_Token*>(this->data)->to_string();
  }
  else
  {
    desc += "NULL";  
  }

  desc += " }";
  return desc;

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
  
  while (!this->is_finished())
  {
    int32_t value = this->peek_char();
    
    if (this->is_whitespace(static_cast<char>(value)))
    {
      this->eat_char();
    }
    else
    {
      return;
    }
  }
}

Parse_Function terminals[] = {
  try_parse_select,
  try_parse_from,
  try_parse_asterisk,
  try_parse_comma,
  // non-terminals
  try_parse_ident,
};

constexpr size_t terminals_length = sizeof(terminals) / sizeof(terminals[0]) ;


Token SQL_Parse_Context::eat_token()
{
  Token token = { .type = NONE, .data = NULL, };

  this->skip_whitespace();

  for (size_t i = 0; i < terminals_length; i++)
  {
    Parse_Function func = terminals[i];
    bool success = false;

    func(this, &token, &success);
  
    if (success)
    {
      this->error = false;
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

    token->type = Token_Type::SELECT;
    *success = true;
    return;
  }
  
  token->type = Token_Type::NONE;
  *success = false;
}

void try_parse_from(SQL_Parse_Context* parser, Token *token, bool *success)
{
  std::string token_identifier = "from";
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

    token->type = Token_Type::FROM;
    *success = true;
    return;
  }
  
  token->type = Token_Type::NONE;
  *success = false;
}

void try_parse_asterisk(SQL_Parse_Context* parser, Token *token, bool *success)
{
  if (parser->peek_char() != '*' || !parser->is_whitespace(parser->peek_n_char(1)))
  {
    token->type = Token_Type::NONE;
    *success = false;
    return;
  }

  parser->eat_char();
  token->type = Token_Type::ASTERISK;
  *success = true;
}

void try_parse_comma(SQL_Parse_Context* parser, Token *token, bool *success)
{
  if (parser->peek_char() != ',')
  {
    token->type = Token_Type::NONE;
    *success = false;
    return;
  }

  parser->eat_char();
  token->type = Token_Type::COMMA;
  *success = true;
}


void try_parse_ident(SQL_Parse_Context* parser, Token *token, bool *success)
{
  size_t i = 0;
  int32_t c = parser->peek_n_char(i);
  while (c != END_OF_SOURCE && !parser->is_whitespace(c) && c != ',')
  {
    if (!isalnum(c))
    {
      token->type = Token_Type::NONE;
      *success = false;
      return;
    }

    i++;
    c = parser->peek_n_char(i);
  }

  std::string ident_name = parser->source.substr(parser->index, i);
  for (size_t j = 0; j < i; j++)
  {
    parser->eat_char();
  }

  token->type = Token_Type::IDENT;

  Ident_Token *ident = new Ident_Token();
  token->data = ident;

  ident->dotted = false;
  ident->quoted = false;
  ident->ident = ident_name;
  
  *success = true;
}

