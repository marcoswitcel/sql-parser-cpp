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
  std::string error_message;

  SQL_Parse_Context(std::string source);

  int32_t peek_char();
  int32_t peek_n_char(size_t n);

  int32_t eat_char();

  Token eat_token();
  Token peek_token();

  Ast_Node* eat_node();
  Expression_Ast_Node* eat_expression_ast_node();
  Binary_Expression_Ast_Node* eat_binary_expression_ast_node();
  Expression_Ast_Node* eat_ident_or_function_call(Token &token);

  /**
   * @brief reporta um erro irrecuperável de parsing
   * 
   * @param error_message mensagem de erro que deve ser apresentada
   */
  void report_error(std::string error_message);

  inline bool is_finished();
  inline bool is_whitespace(char value);
  inline void skip_whitespace();
};

/**
 * @brief Função que realiza o parse de tokens, atualiza o estado do parser caso haja sucesso
 * sinaliza via `success`.
 * 
 */
typedef void (*Parse_Function)(SQL_Parse_Context* parser, Token* token, bool* success);

void try_parse_describe(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_select(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_from(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_where(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_asterisk(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_equals(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_not_equals(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_like(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_not(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_as(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_greater_than(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_lower_than(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_comma(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_semicolon(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_open_parenthesis(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_close_parenthesis(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_or(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_and(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_concat(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_string(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_number(SQL_Parse_Context* parser, Token *token, bool *success);
void try_parse_ident(SQL_Parse_Context* parser, Token *token, bool *success);
