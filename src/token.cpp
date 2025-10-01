#pragma once

#include <string>
#include <assert.h>

#include "./token.hpp"

std::string get_description(Token_Type &token_type)
{
  switch (token_type)
  {
    case Token_Type::None: return "None";
    case Token_Type::Describe: return "Describe";
    case Token_Type::Select: return "Select";
    case Token_Type::From: return "From";
    case Token_Type::Asterisk: return "Asterisk";
    case Token_Type::Equals: return "Equals";
    case Token_Type::Not_Equals: return "Not_Equals";
    case Token_Type::Like: return "Like";
    case Token_Type::Not: return "Not";
    case Token_Type::As: return "As";
    case Token_Type::Greater_Than: return "Greater_Than";
    case Token_Type::Lower_Than: return "Lower_Than";
    case Token_Type::Comma: return "Comma";
    case Token_Type::Semicolon: return "Semicolon";
    case Token_Type::Open_Parenthesis: return "Open_Parenthesis";
    case Token_Type::Close_Parenthesis: return "Close_Parenthesis";
    case Token_Type::And: return "And";
    case Token_Type::Or: return "Or";
    case Token_Type::String: return "String";
    case Token_Type::Number: return "Number";
    case Token_Type::Ident: return "Ident";
    case Token_Type::Where: return "Where";
  }

  assert(false);

  // @note Talvez retornar o valor junto? castear para number e printar
  return "[Invalid Token_Type]";
}

std::string Ident_Token::to_string()
{
  return "Ident_Token { .ident = '" + this->ident + "' }";
}

std::string String_Token::to_string()
{
  return "String_Token { .value = \"" + this->value + "\" }";
}

std::string Number_Token::to_string()
{
  return "Number_Token { .value = " + std::to_string(this->value) + " }";
}

std::string Token::to_string()
{
  std::string desc = "Token { .type = " + get_description(this->type) + ", .data = ";

  if (this->data && this->type == Token_Type::Ident)
  {
    desc += static_cast<Ident_Token*>(this->data)->to_string();
  }
  else if (this->data && this->type == Token_Type::String)
  {
    desc += static_cast<String_Token*>(this->data)->to_string();
  }
  else if (this->data && this->type == Token_Type::Number)
  {
    desc += static_cast<Number_Token*>(this->data)->to_string();
  }
  else
  {
    desc += "NULL";  
  }

  desc += " }";
  return desc;

}
