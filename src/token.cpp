#pragma once

#include <assert.h>

#include "./token.hpp"

std::string get_description(Token_Type &token_type)
{
  switch (token_type)
  {
    case Token_Type::None: return "None";
    case Token_Type::Select: return "Select";
    case Token_Type::From: return "From";
    case Token_Type::Asterisk: return "Asterisk";
    case Token_Type::Equals: return "Equals";
    case Token_Type::Greater_Than: return "Greater_Than";
    case Token_Type::Lower_Than: return "Lower_Than";
    case Token_Type::Comma: return "Comma";
    case Token_Type::String: return "String";
    case Token_Type::Ident: return "Ident";
    case Token_Type::Where: return "Where";
  }

  assert(false);

  // @note Talvez retornar o valor junto? castear para number e printar
  return "[Invalid Token_Type]";
}

