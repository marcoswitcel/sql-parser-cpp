#pragma once

#include <string>
#include <assert.h>
#include <cstdint>
#include <cctype>

#include "./sql-parse-context.hpp"
#include "./token.cpp"


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

      if (token.type == Token_Type::Ident)
      {
        auto ident = std::make_shared<Ident_Expression_Ast_Node>();
        ident.get()->ident_name = static_cast<Ident_Token*>(token.data)->ident;
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
              // @todo João, terminar aqui... tentar um método try_eat_expression_node ou coisa parecida...
              select->where = std::unique_ptr<Where_Ast_Node>(new Where_Ast_Node());

              Binary_Expression_Ast_Node* bin_exp = this->eat_binary_expression_ast_node();
              // @todo João, em caso de nullo deveria retornar nullo pra sinalizar o erro, por hora, pelo menos...
              if (bin_exp)
              {
                select->where->conditions.push_back(std::unique_ptr<Binary_Expression_Ast_Node>(bin_exp));
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
  
  Trace("node: %s", node->to_string().c_str());
  return node;
}

Parse_Function terminals[] = {
  try_parse_select,
  try_parse_from,
  try_parse_where,
  try_parse_asterisk,
  try_parse_equals,
  try_parse_greater_than,
  try_parse_lower_than,
  try_parse_comma,
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
      this->error = false;
      return token;
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


void try_parse_ident(SQL_Parse_Context* parser, Token *token, bool *success)
{
  size_t i = 0;
  int32_t c = parser->peek_n_char(i);
  // @todo João, melhorar para não ter dependência com o símbolo ','
  while (c != END_OF_SOURCE && !parser->is_whitespace(c) && c != ',')
  {
    if (!isalnum(c))
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

