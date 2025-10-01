#pragma once

#include <string>
#include <cstdint>
#include <string>


enum class Token_Type {
  None, // não inicializado
  Describe,
  Select,
  From,
  Asterisk,
  Equals,
  Not_Equals,
  Like,
  Not,
  As,
  Comma,
  Semicolon,
  Open_Parenthesis,
  Close_Parenthesis,
  Where,
  Greater_Than,
  Lower_Than,
  And,
  Or,
  // non-terminal
  String,
  Number,
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

struct String_Token
{
  std::string value;
  
  std::string to_string();
};

struct Number_Token
{
  int64_t value;
  
  std::string to_string();
};


/**
 * @brief 
 * @note João, analisar converter token para um classe abstrata e instanciar os subtypes,
 * só que por hora a maioria dos subtypes não teria nenhum field...
 */
struct Token
{
  Token_Type type;
  void* data;

  std::string to_string();
};

