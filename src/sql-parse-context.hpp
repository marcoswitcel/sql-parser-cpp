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

std::string get_description(Token_Type &token_type);


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
  /* union {
    Select_Token select;
    From_Token from;
    Asterisk_Token asterisk;
    Ident_Token ident;
  } data; */

  std::string to_string();
};



constexpr int32_t END_OF_SOURCE = -9999;

struct SQL_Parse_Context
{
  std::string source;
  uint64_t index = 0;
  bool error = false;

  SQL_Parse_Context(std::string source);

  int32_t peek_char();
  int32_t peek_n_char(size_t n);

  int32_t eat_char();

  Token eat_token();

  inline bool is_finished();
  inline bool is_whitespace(char value);
  inline void skip_whitespace();
};

typedef void (*Parse_Function)(SQL_Parse_Context*, Token *, bool *);

void try_parse_select(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_from(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_asterisk(SQL_Parse_Context* parser, Token *token, bool *success);
