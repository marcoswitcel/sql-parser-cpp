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

