#pragma once

#include <string>
#include <cstdint>

enum Token_Type {
  NONE, // não inicializado
  SELECT,
  FROM,
  ASTERISK,
  COMMA,
  // non-terminal
  IDENT
};

std::string get_description(Token_Type &token_type);


struct Ident_Token
{
  std::string ident;
  bool quoted;
  bool dotted;
  
  std::string to_string();
};


struct Token
{
  Token_Type type;
  void* data;

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

struct Select_Node
{
  std::vector<Ident_Token> fields;
  std::string from;
};

typedef void (*Parse_Function)(SQL_Parse_Context*, Token *, bool *);

void try_parse_select(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_from(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_asterisk(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_comma(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_ident(SQL_Parse_Context* parser, Token *token, bool *success);
