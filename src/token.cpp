#pragma once

#include "assert.h"

#include "./token.hpp"

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
    case WHERE: return "WHERE";
  }

  assert(false);

  // @note Talvez retornar o valor junto? castear para number e printar
  return "[Invalid Token_Type]";
}

