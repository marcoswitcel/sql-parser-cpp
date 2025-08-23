#pragma once

#include <string>
#include <cstdint>

enum Token_Type {
  NONE, // não inicializado
  SELECT,
  FROM,
  ASTERISK,
  // non-terminal
  IDENT
};

struct None {};

struct Select_Token {};

struct From_Token {};

struct Asterisk_Token {};

struct Ident_Token
{
  std::string ident;
  bool quoted;
  bool dotted;
};


struct Token
{
  Token_Type type;
  union {
    Select_Token select;
    From_Token from;
    Asterisk_Token asterisk;
    Ident_Token ident;
  } data;
};

// typedef void (*Parse_Function)(SQL_Parse_Context*, Token *, bool *);

// @todo joão, @cleanup
//void try_parse_select(SQL_Parse_Context* parser, Token *token, bool *success);


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

  inline bool is_whitespace(char value)
  {
    return (value == ' ' || value == '\t' || value == '\r' || value == '\n');
  }


  void skip_whitespace()
  {
    
    while (this->is_finished())
    {
      int32_t value = this->peek_char();
      if (value < 0) return;

      if (!this->is_whitespace(static_cast<char>(value))) return;
    }
  }


  Token eat_token()
  {
    this->skip_whitespace();

    bool success = false;
    token.type = NONE;

    Parse_Function func = try_parse_select(this, &token, &success);

    return token;
  }
};


void try_parse_select(SQL_Parse_Context* parser, Token *token, bool *success)
{

}