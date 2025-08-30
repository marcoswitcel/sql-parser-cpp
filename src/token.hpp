#pragma once

#include <string>
#include <cstdint>


enum class Token_Type {
  None, // não inicializado
  Select,
  From,
  Asterisk,
  Comma,
  Where,
  // non-terminal
  Ident
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

