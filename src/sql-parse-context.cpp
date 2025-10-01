#pragma once

#include <string>
#include <assert.h>
#include <cstdint>
#include <cctype>

#include "./sql-parse-context.hpp"
#include "./token.cpp"


SQL_Parse_Context::SQL_Parse_Context(std::string source)
{
  this->source = source;
  this->index = 0;
}

int32_t SQL_Parse_Context::peek_char()
{
  if (this->is_finished()) return END_OF_SOURCE;

  return static_cast<int32_t>(this->source[this->index]);
}

int32_t SQL_Parse_Context::peek_n_char(size_t n)
{
  size_t index_with_n = this->index + n;

  if (index_with_n >= this->source.length()) return END_OF_SOURCE;

  return static_cast<int32_t>(this->source[index_with_n]);
}



int32_t SQL_Parse_Context::eat_char()
{
  if (this->is_finished()) return END_OF_SOURCE;

  return static_cast<int32_t>(this->source[this->index++]);
}

inline bool SQL_Parse_Context::is_finished()
{
  return this->index >= this->source.length();
}

inline bool SQL_Parse_Context::is_whitespace(char value)
{
  return (value == ' ' || value == '\t' || value == '\r' || value == '\n');
}


void SQL_Parse_Context::skip_whitespace()
{
  
  while (!this->is_finished())
  {
    int32_t value = this->peek_char();
    
    if (this->is_whitespace(static_cast<char>(value)))
    {
      this->eat_char();
    }
    else
    {
      return;
    }
  }
}

Ast_Node* SQL_Parse_Context::eat_node()
{
  Token token = this->eat_token();
  
  if (this->error)
  {
    return NULL;
  }
  
  if (token.type == Token_Type::Select)
  {
    auto select = new Select_Ast_Node();

    this->skip_whitespace();

    while (!this->is_finished())
    {
      Token token = this->eat_token();

      if (this->error)
      {
        return NULL;
      }

      if (token.type == Token_Type::Ident || token.type == Token_Type::Asterisk)
      {
        auto ident = std::make_shared<Ident_Expression_Ast_Node>();
        if (token.type == Token_Type::Ident)
        {
          ident.get()->ident_name = static_cast<Ident_Token*>(token.data)->ident;
        }
        else
        {
          assert(token.type == Token_Type::Asterisk);
          ident.get()->ident_name = "*";
        }
        select->fields.push_back(ident);

        token = this->eat_token();

        if (token.type == Token_Type::From)
        {
          token = this->eat_token();

          if (token.type == Token_Type::Ident)
          {
            select->from = std::unique_ptr<From_Ast_Node>(new From_Ast_Node());
            select->from.get()->ident_name = static_cast<Ident_Token*>(token.data)->ident;

            token = this->eat_token();

            if (token.type == Token_Type::Where)
            {
              select->where = std::unique_ptr<Where_Ast_Node>(new Where_Ast_Node());

              Binary_Expression_Ast_Node* bin_exp = this->eat_binary_expression_ast_node();
              // @todo João, em caso de nullo deveria retornar nullo pra sinalizar o erro, por hora, pelo menos...
              if (bin_exp)
              {
                select->where->conditions = std::unique_ptr<Binary_Expression_Ast_Node>(bin_exp);
              }

              return select;
            }
            else if (this->is_finished())
            {
              return select;
            }
            else
            {
              // erros de parsing ou outros tokens vão aqui...
              return NULL;
            }
          }
          else
          {
            return NULL;
          }
        }
        else if (token.type == Token_Type::Comma)
        {

        }
        else
        {
          return NULL;  
        }
      }
      else
      {
        return NULL;
      }
    
      this->skip_whitespace();
    }

    return select;
  }
  else if (token.type == Token_Type::Describe)
  {
    auto describe = new Describe_Ast_Node();

    Token token = this->eat_token();

    if (this->error)
    {
      return NULL;
    }

    if (token.type == Token_Type::Ident)
    {
      describe->ident_name = static_cast<Ident_Token*>(token.data)->ident;
      
      return describe;
    }

    return NULL;
  }
  
  return NULL;
}

Binary_Expression_Ast_Node* SQL_Parse_Context::eat_binary_expression_ast_node()
{
  Binary_Expression_Ast_Node* node = new Binary_Expression_Ast_Node();
  
  while (!node->left || !node->right || node->op.size() == 0)
  {
    this->skip_whitespace();
    Token token = this->eat_token();

    if (token.type == Token_Type::Ident)
    {
      auto ident = new Ident_Expression_Ast_Node();
      ident->ident_name = static_cast<Ident_Token*>(token.data)->ident;
  
      if (node->left == NULL)
      {
        node->left = std::unique_ptr<Ident_Expression_Ast_Node>(ident);
      }
      else if (node->right == NULL)
      {
        node->right = std::unique_ptr<Ident_Expression_Ast_Node>(ident);
      }
      else
      {
        delete ident;
        this->error = true;
        return NULL;
      }
    }
    else if (token.type == Token_Type::Equals)
    {
      // @todo João, falta vários operadores...
      node->op = "=";
    }
    else if (token.type == Token_Type::Not_Equals)
    {
      node->op = "<>";
    }
    else if (token.type == Token_Type::Like)
    {
      node->op = "like";
    }
    else if (token.type == Token_Type::And)
    {
      node->op = "and";
    }
    else if (token.type == Token_Type::Or)
    {
      node->op = "or";
    }
    else if (token.type == Token_Type::String)
    {
      auto string_value = new String_Literal_Expression_Ast_Node();
      string_value->value = static_cast<String_Token*>(token.data)->value;
  
  
      if (node->left == NULL)
      {
        node->left = std::unique_ptr<String_Literal_Expression_Ast_Node>(string_value);
      }
      else if (node->right == NULL)
      {
        node->right = std::unique_ptr<String_Literal_Expression_Ast_Node>(string_value);
      }
      else
      {
        this->error = true;
        return NULL;
      }
    }
    else
    {
      // @todo João, leak on return, serve para os returns acima também...
      this->error = true;
      return NULL;
    }
  }

  this->skip_whitespace();
  Token token = this->eat_token();

  if (token.type == Token_Type::Or)
  {
    Binary_Expression_Ast_Node* new_root_node = new Binary_Expression_Ast_Node();
    new_root_node->op = "or";
    new_root_node->left = std::unique_ptr<Binary_Expression_Ast_Node>(node);

    new_root_node->right = std::unique_ptr<Binary_Expression_Ast_Node>(this->eat_binary_expression_ast_node());

    node = new_root_node;
  }
  else if (token.type == Token_Type::And)
  {
    Binary_Expression_Ast_Node* new_root_node = new Binary_Expression_Ast_Node();
    new_root_node->op = "and";
    new_root_node->left = std::unique_ptr<Binary_Expression_Ast_Node>(node);

    new_root_node->right = std::unique_ptr<Binary_Expression_Ast_Node>(this->eat_binary_expression_ast_node());

    node = new_root_node;
  }
  
  return node;
}

Parse_Function terminals[] = {
  try_parse_describe,
  try_parse_select,
  try_parse_from,
  try_parse_where,
  try_parse_asterisk,
  try_parse_equals,
  try_parse_not_equals,
  try_parse_like,
  try_parse_not,
  try_parse_greater_than,
  try_parse_lower_than,
  try_parse_comma,
  try_parse_semicolon,
  try_parse_open_parenthesis,
  try_parse_close_parenthesis,
  try_parse_or,
  try_parse_and,
  // non-terminals
  try_parse_string,
  try_parse_number,
  try_parse_ident,
};

constexpr size_t terminals_length = sizeof(terminals) / sizeof(terminals[0]) ;


Token SQL_Parse_Context::eat_token()
{
  Token token = { .type = Token_Type::None, .data = NULL, };

  this->skip_whitespace();

  for (size_t i = 0; i < terminals_length; i++)
  {
    Parse_Function func = terminals[i];
    bool success = false;

    func(this, &token, &success);
  
    if (success)
    {
      assert(success && !this->error);
      this->error = false;
      return token;
    } else if (this->error) {
      break;
    }
  }

  this->error = true;
  return token;
}

static inline bool try_consume_keyword(SQL_Parse_Context *parser, std::string token_identifier)
{
  const auto index_after_last_char = token_identifier.size();
  
  bool equal = true;
  for (size_t i = 0; i < token_identifier.size(); i++)
  {
    char c = std::tolower(parser->peek_n_char(i));
    if (c != token_identifier.at(i))
    {
      equal = false;
      break;
    }
  }

  if (equal &&
    parser->peek_n_char(index_after_last_char) != END_OF_SOURCE &&
    parser->is_whitespace(parser->peek_n_char(index_after_last_char)))
  {
    for (size_t i = 0; i < token_identifier.size(); i++)
    {
      parser->eat_char();
    }

    return true;
  }

  return false;
}

void try_parse_describe(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "describe");
  
  if (is_consumed)
  {
    token->type = Token_Type::Describe;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_select(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "select");
  
  if (is_consumed)
  {
    token->type = Token_Type::Select;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_and(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "and");
  
  if (is_consumed)
  {
    token->type = Token_Type::And;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_or(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "or");
  
  if (is_consumed)
  {
    token->type = Token_Type::Or;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_from(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "from"); 
  
  if (is_consumed)
  {
    token->type = Token_Type::From;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_where(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "where"); 
  
  if (is_consumed)
  {
    token->type = Token_Type::Where;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_asterisk(SQL_Parse_Context* parser, Token *token, bool *success)
{
  if (parser->peek_char() != '*')
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }

  parser->eat_char();
  token->type = Token_Type::Asterisk;
  *success = true;
}

void try_parse_equals(SQL_Parse_Context* parser, Token *token, bool *success)
{
  if (parser->peek_char() != '=')
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }

  parser->eat_char();
  token->type = Token_Type::Equals;
  *success = true;
}

void try_parse_not_equals(SQL_Parse_Context* parser, Token *token, bool *success)
{
  // @note João, não era bem pra isso que fiz essa função `try_consume_keyword`, mas enfim... o 'diferente' é multicaracter
  bool is_consumed = try_consume_keyword(parser, "<>"); 
  
  if (is_consumed)
  {
    token->type = Token_Type::Not_Equals;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_like(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "like"); 
  
  if (is_consumed)
  {
    token->type = Token_Type::Like;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_not(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "not"); 
  
  if (is_consumed)
  {
    token->type = Token_Type::Not;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_greater_than(SQL_Parse_Context* parser, Token *token, bool *success)
{
  if (parser->peek_char() != '>')
  {
    token->type = Token_Type::None;;
    *success = false;
    return;
  }

  parser->eat_char();
  token->type = Token_Type::Greater_Than;
  *success = true;
}

void try_parse_lower_than(SQL_Parse_Context* parser, Token *token, bool *success)
{
  if (parser->peek_char() != '<')
  {
    token->type = Token_Type::None;;
    *success = false;
    return;
  }

  parser->eat_char();
  token->type = Token_Type::Lower_Than;
  *success = true;
}

void try_parse_comma(SQL_Parse_Context* parser, Token *token, bool *success)
{
  if (parser->peek_char() != ',')
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }

  parser->eat_char();
  token->type = Token_Type::Comma;
  *success = true;
}

void try_parse_semicolon(SQL_Parse_Context* parser, Token *token, bool *success)
{
  if (parser->peek_char() != ';')
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }

  parser->eat_char();
  token->type = Token_Type::Semicolon;
  *success = true;
}

void try_parse_open_parenthesis(SQL_Parse_Context* parser, Token *token, bool *success)
{
  if (parser->peek_char() != '(')
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }

  parser->eat_char();
  token->type = Token_Type::Open_Parenthesis;
  *success = true;
}

void try_parse_close_parenthesis(SQL_Parse_Context* parser, Token *token, bool *success)
{
  if (parser->peek_char() != ')')
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }

  parser->eat_char();
  token->type = Token_Type::Close_Parenthesis;
  *success = true;
}


void try_parse_string(SQL_Parse_Context* parser, Token *token, bool *success)
{
  const char quote = '\'';
  size_t i = 0;
  int32_t c = parser->peek_n_char(i);

  if (c != quote)
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }

  // contabiliza aspa de abertuta e busca próximo char
  c = parser->peek_n_char(++i);

  while (c != END_OF_SOURCE && c != quote)
  {
    i++;
    c = parser->peek_n_char(i);
  }

  // contabiliza a aspa de fehcamento
  i++;

  // pula aspa de abertura e desconta as duas aspas do tamanho da string
  std::string value = parser->source.substr(parser->index + 1, i - 2);
  // consome todos caracteres
  for (size_t j = 0; j < i; j++)
  {
    parser->eat_char();
  }

  token->type = Token_Type::String;

  String_Token *ident = new String_Token();
  token->data = ident;

  ident->value = value;
  
  *success = true;
}

inline bool is_digit(char c)
{
  return '0' <= c && c <= '9';
}

inline bool is_valid_ident_char(char c)
{
  return isalnum(c) || c == '_';
}

void try_parse_number(SQL_Parse_Context* parser, Token *token, bool *success)
{
  size_t i = 0;
  int32_t c = parser->peek_n_char(i);

  // @todo João, ainda falta bloquear números com zero a esquerda, exceto o zero...
  if (c == END_OF_SOURCE || parser->is_whitespace(c))
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }

  if (c == '0' && is_digit(parser->peek_n_char(i+1)))
  {
    // @todo João, falta a habilitade de sinalizar um erro impeditivo de tokenização.
    // Acredito que faria sentido usar o atributo de 'error' para finalizar a tokenização.
    // Seria necessário ajustar apenas no loop do método `eat_token`.
    // @note Implementado, falta validar se vai atender, senão reverter.
    token->type = Token_Type::None;
    *success = false;
    parser->error = true;
    return;
  }

  while (c != END_OF_SOURCE && !parser->is_whitespace(c) && c != ',')
  {
    if (!is_digit(c))
    {
      token->type = Token_Type::None;
      *success = false;
      return;
    }
    
    i++;
    c = parser->peek_n_char(i);
  }
  
  std::string value = parser->source.substr(parser->index, i);
  // consome todos caracteres
  for (size_t j = 0; j < i; j++)
  {
    parser->eat_char();
  }

  token->type = Token_Type::Number;

  Number_Token *ident = new Number_Token();
  token->data = ident;
  
  try 
  {
    ident->value = std::stod(value);
  } catch (std::invalid_argument& ex)
  {
    // @note João, avaliar melhor esses dois catchs
    assert(false);
  } catch (std::out_of_range& ex)
  {
    assert(false);
  }
  
  *success = true;
}

// @todo João, incluir àspas dupals para permitir parsear identificadores com caracteres especiais (aparentemente ANSI é a áspa dupla)
void try_parse_ident(SQL_Parse_Context* parser, Token *token, bool *success)
{
  size_t i = 0;
  int32_t c = parser->peek_n_char(i);
  
  // não pode começar com esse caracteres
  if (c == '_' || is_digit(c))
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }
  
  // @todo João, melhorar para não ter dependência com o símbolo ','
  while (c != END_OF_SOURCE && !parser->is_whitespace(c) && c != ',')
  {
    if (!is_valid_ident_char(c))
    {
      token->type = Token_Type::None;
      *success = false;
      return;
    }

    i++;
    c = parser->peek_n_char(i);
  }

  std::string ident_name = parser->source.substr(parser->index, i);
  for (size_t j = 0; j < i; j++)
  {
    parser->eat_char();
  }

  token->type = Token_Type::Ident;

  Ident_Token *ident = new Ident_Token();
  token->data = ident;

  ident->dotted = false;
  ident->quoted = false;
  ident->ident = ident_name;
  
  *success = true;
}

