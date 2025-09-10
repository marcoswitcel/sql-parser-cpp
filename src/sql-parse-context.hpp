#pragma once

#include <string>
#include <cstdint>

#include "./token.hpp"
#include "./ast_node.hpp"


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

  Ast_Node* eat_node();
  Binary_Expression_Ast_Node* eat_binary_expression_ast_node();

  inline bool is_finished();
  inline bool is_whitespace(char value);
  inline void skip_whitespace();
};

typedef void (*Parse_Function)(SQL_Parse_Context*, Token *, bool *);

void try_parse_select(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_from(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_where(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_asterisk(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_equals(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_greater_than(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_lower_than(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_comma(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_or(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_and(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_string(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_number(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_ident(SQL_Parse_Context* parser, Token *token, bool *success);
// @todo João, falta: number_literal, string_literal, equals (e afins) para poder parsear mais expressões além de identificadores
