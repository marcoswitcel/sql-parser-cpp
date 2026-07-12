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
      // Token token = this->eat_token();

      if (this->error)
      {
        return NULL;
      }

      Expression_Ast_Node* expression_node = this->eat_expression_ast_node();

      // @todo joão, pra parsear uma "Expression_Ast_Node" aqui vou precisar implementar um comando peek_token e
      // refatorar o método `eat_binary_expression_ast_node` para usar o peek_token ao invés de tentar consumir os tokens
      // @note atualizado: avaliar usar o método peek_token implemetando e talvez não precise refatorar o método `eat_binary_expression_ast_node` ainda
      // @note atualizado: talvez fosse melhor só fazer o eat_token e reverter se der erro? um mecanismo de revert automático seria melhor que um método peek_n_token?
      // Pergunto isso porque um peek_n_token apresenta várias complexidades, como, parsear token a token e armazenar num buffer? e se der erro? armazenar em alguma
      // estrutura? 
      if (expression_node && (expression_node->type == Ast_Node_Type::Ident_Expression_Ast_Node || expression_node->type == Ast_Node_Type::String_Literal_Expression_Ast_Node || expression_node->type == Ast_Node_Type::Number_Literal_Expression_Ast_Node || expression_node->type == Ast_Node_Type::Binary_Expression_Ast_Node || expression_node->type == Ast_Node_Type::Function_Call_Expression_Ast_Node))
      {
        select->fields.push_back(std::shared_ptr<Expression_Ast_Node>(expression_node));
        
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
              
              if (bin_exp)
              {
                select->where->conditions = std::unique_ptr<Binary_Expression_Ast_Node>(bin_exp);
              }
              else
              {
                this->report_error("Não pode parsear a expressão após Where");
                return NULL;
              }
            }

            if (token.type == Token_Type::Group || this->last_eaten_token.type == Token_Type::Group)
            {
              token = this->eat_token();

              if (token.type == Token_Type::By)
              {
                select->group_by = std::unique_ptr<Group_By_Ast_Node>(new Group_By_Ast_Node());
                
                bool expect_comma = false;
                while (!this->is_finished())
                {
                  token = this->eat_token();

                  if (token.type == Token_Type::Ident && !expect_comma)
                  {
                    auto ident_name = new Ident_Expression_Ast_Node();
                    ident_name->ident_name =  static_cast<Ident_Token*>(token.data)->ident;

                    select->group_by->groups.push_back(std::unique_ptr<Expression_Ast_Node>(ident_name));

                    expect_comma = true;
                  }
                  else if (token.type == Token_Type::Comma && expect_comma)
                  {
                    expect_comma = false;
                  }
                  else
                  {
                    this->report_error("Token inválido depois no Group By: " + get_description(token.type));
                    return NULL;
                  }

                  this->skip_whitespace();
                }

                if (!expect_comma)
                {
                  this->report_error("Vírgula sobrando");
                  return NULL;
                }
                
              }
              else
              {
                this->report_error("Token inválido depois de Group: " + get_description(token.type));
                return NULL;
              }
            }

            if (token.type == Token_Type::Order || this->last_eaten_token.type == Token_Type::Order)
            {
              token = this->eat_token();

              if (token.type == Token_Type::By)
              {
                // @todo João, pendente concluir aqui... @wip
              }
              else
              {
                this->report_error("Token inválido depois de Order: " + get_description(token.type));
                return NULL;
              }

              this->skip_whitespace();
            }
            
            if (this->is_finished())
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
          // apenas consome e no final do bloco faz o skip do whitespace
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
      describe->ident_name = std::unique_ptr<Ident_Expression_Ast_Node>(new Ident_Expression_Ast_Node());
      describe->ident_name->ident_name =  static_cast<Ident_Token*>(token.data)->ident;
      
      return describe;
    }

    return NULL;
  }
  
  return NULL;
}

/**
 * @brief função que parseia um ident ou uma chamada de função
 * @note Deve ser chamada apenas após ter consumido o Token do tipo Ident
 * 
 * @param token Ident token consumido
 * @return Expression_Ast_Node* `Function_Call_Expression_Ast_Node`, `Ident_Expression_Ast_Node` ou NULL em caso de erros
 */
Expression_Ast_Node* SQL_Parse_Context::eat_ident_or_function_call(Token &token)
{
  Expression_Ast_Node* expression = NULL;

  Token next_token = this->peek_token();
  if (next_token.type == Token_Type::Open_Parenthesis)
  {
    this->eat_token();
    
    std::vector<Expression_Ast_Node*> argument_list;
    bool closed_parenthesis = false;
    
    this->skip_whitespace();
    while (!this->is_finished())
    {
      if (this->error)
      {
        return NULL;
      }

      if (this->peek_token().type == Token_Type::Close_Parenthesis)
      {
        this->eat_token();
        closed_parenthesis = true;
        break;
      }

      Expression_Ast_Node* expression_node = this->eat_expression_ast_node();

      if (expression_node && (expression_node->type == Ast_Node_Type::Ident_Expression_Ast_Node || expression_node->type == Ast_Node_Type::String_Literal_Expression_Ast_Node || expression_node->type == Ast_Node_Type::Number_Literal_Expression_Ast_Node || expression_node->type == Ast_Node_Type::Binary_Expression_Ast_Node || expression_node->type == Ast_Node_Type::Function_Call_Expression_Ast_Node))
      {
        argument_list.push_back(expression_node);
      }

      if (this->peek_token().type == Token_Type::Comma)
      {
        this->eat_token();
      }

      this->skip_whitespace();
    }

    if (!closed_parenthesis) return NULL;

    auto call = new Function_Call_Expression_Ast_Node();
    call->name = static_cast<Ident_Token*>(token.data)->ident;
    call->argument_list = argument_list;
    expression = call;
  }
  else 
  {
    auto ident = new Ident_Expression_Ast_Node();
    ident->ident_name = static_cast<Ident_Token*>(token.data)->ident;
    expression = ident;  
  }

  return expression;
}

Expression_Ast_Node* SQL_Parse_Context::eat_expression_ast_node()
{
  Token token = this->eat_token();

  if (token.type != Token_Type::Ident && token.type != Token_Type::Asterisk && token.type != Token_Type::String && token.type != Token_Type::Number) return NULL;

  Expression_Ast_Node* expression = NULL;

  if (token.type == Token_Type::Ident)
  {
    expression = this->eat_ident_or_function_call(token);

    if (expression == NULL) return NULL;
  }
  else if (token.type == Token_Type::String)
  {
    auto string = new String_Literal_Expression_Ast_Node();
    string->value = static_cast<String_Token*>(token.data)->value;
    expression = string;
  }
  else if (token.type == Token_Type::Number)
  {
    auto number = new Number_Literal_Expression_Ast_Node();
    number->value = static_cast<Number_Token*>(token.data)->value;
    expression = number;
  }
  else
  {
    assert(token.type == Token_Type::Asterisk);
    auto ident = new Ident_Expression_Ast_Node();
    ident->ident_name = "*";
    expression = ident;
  }

  assert(expression);

  token = this->peek_token();

  if (token.type == Token_Type::As)
  {
    // consome "As" token
    this->eat_token(); 

    token = this->peek_token();

    if (token.type == Token_Type::Ident)
    {
      // consome "Ident" token
      this->eat_token(); 
      expression->as = static_cast<Ident_Token*>(token.data)->ident;
    }
    else
    {
      return NULL;
    }
  }
  else if (token.type == Token_Type::Concat)
  {
    // consome "Concat" token
    this->eat_token(); 

    auto bin_exp = new Binary_Expression_Ast_Node();
    bin_exp->left = std::unique_ptr<Expression_Ast_Node>(expression);
    bin_exp->op = "concat";
    auto bin_exp_right = this->eat_expression_ast_node();

    if (bin_exp_right == NULL) return NULL;

    if (!bin_exp_right->as.empty())
    {
      bin_exp->as = bin_exp_right->as;
      bin_exp_right->as = "";
    }

    bin_exp->right = std::unique_ptr<Expression_Ast_Node>(bin_exp_right);

    expression = bin_exp;
  }

  return expression;
}

/**
 * @brief 
 * @todo joão, a função atual não garante a ordem correta... "a > 2" e "> a 2" funcionam...
 * @return Binary_Expression_Ast_Node* 
 */
Binary_Expression_Ast_Node* SQL_Parse_Context::eat_binary_expression_ast_node()
{
  Binary_Expression_Ast_Node* node = new Binary_Expression_Ast_Node();
  
  bool found_and_not_consumed_not_keyword = false;
  while (!node->left || !node->right || node->op.size() == 0)
  {
    this->skip_whitespace();
    Token token = this->eat_token();

    if (found_and_not_consumed_not_keyword && token.type != Token_Type::Like)
    {
      this->error = true;
      return NULL;
    }

    if (token.type == Token_Type::Ident)
    {
      Expression_Ast_Node* expression = this->eat_ident_or_function_call(token);

      if (expression == NULL) return NULL;

      if (node->left == NULL)
      {
        node->left = std::unique_ptr<Expression_Ast_Node>(expression);
      }
      else if (node->right == NULL)
      {
        node->right = std::unique_ptr<Expression_Ast_Node>(expression);
      }
      else
      {
        delete expression;
        this->error = true;
        return NULL;
      }
    }
    else if (token.type == Token_Type::Equals)
    {
      node->op = "=";
    }
    else if (token.type == Token_Type::Not_Equals)
    {
      node->op = "<>";
    }
    else if (token.type == Token_Type::Not)
    {
      if (found_and_not_consumed_not_keyword)
      {
        // @note João, não sei se realmente é um erro usar "NOT NOT", acho que não, mas está bloqueado por hora...
        this->error = true;
        return NULL;
      }
      found_and_not_consumed_not_keyword = true;
    }
    else if (token.type == Token_Type::Like)
    {
      node->op = (found_and_not_consumed_not_keyword) ? "not like" : "like";
      found_and_not_consumed_not_keyword = false;
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
    else if (token.type == Token_Type::Number)
    {
      auto number_value = new Number_Literal_Expression_Ast_Node();
      number_value->value = static_cast<Number_Token*>(token.data)->value;
  
  
      if (node->left == NULL)
      {
        node->left = std::unique_ptr<Number_Literal_Expression_Ast_Node>(number_value);
      }
      else if (node->right == NULL)
      {
        node->right = std::unique_ptr<Number_Literal_Expression_Ast_Node>(number_value);
      }
      else
      {
        this->report_error("Sem espaço para inserir o número");
        return NULL;
      }
    }
    else if (token.type == Token_Type::Lower_Than)
    {
      node->op = "<";
    }
    else if (token.type == Token_Type::Greater_Than)
    {
      node->op = ">";
    }
    else
    {
      // @todo João, leak on return, serve para os returns acima também...
      this->report_error("Operação não suportada ainda");
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
  try_parse_as,
  try_parse_greater_than,
  try_parse_lower_than,
  try_parse_comma,
  try_parse_semicolon,
  try_parse_open_parenthesis,
  try_parse_close_parenthesis,
  try_parse_or,
  try_parse_and,
  try_parse_concat,
  try_parse_group,
  try_parse_by,
  try_parse_order,
  try_parse_asc,
  try_parse_desc,
  // non-terminals
  try_parse_number,
  try_parse_string,
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
      this->last_eaten_token = token;
      return token;
    } else if (this->error) {
      break;
    }
  }

  this->error = true;
  return token;
}

/**
 * @brief peek token
 * 
 * @return Token 
 */
Token SQL_Parse_Context::peek_token()
{
  // salva index
  auto index = this->index;

  // tenta consumir o token
  Token token = this->eat_token();

  // reverte o index para "desfazer" o avanço, mas mantém o estado
  // do campo que sinaliza se houve erro
  this->index = index;

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

void try_parse_concat(SQL_Parse_Context* parser, Token *token, bool *success)
{
  // @note João, não era bem pra isso que fiz essa função `try_consume_keyword`, mas enfim... o 'diferente' é multicaracter
  bool is_consumed = try_consume_keyword(parser, "||"); 
  
  if (is_consumed)
  {
    token->type = Token_Type::Concat;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_group(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "group");
  
  if (is_consumed)
  {
    token->type = Token_Type::Group;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_by(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "by");
  
  if (is_consumed)
  {
    token->type = Token_Type::By;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_order(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "order");
  
  if (is_consumed)
  {
    token->type = Token_Type::Order;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_asc(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "asc");
  
  if (is_consumed)
  {
    token->type = Token_Type::Asc;
    *success = true;
    return;
  }
  
  token->type = Token_Type::None;
  *success = false;
}

void try_parse_desc(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "desc");
  
  if (is_consumed)
  {
    token->type = Token_Type::Desc;
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

void try_parse_as(SQL_Parse_Context* parser, Token *token, bool *success)
{
  bool is_consumed = try_consume_keyword(parser, "as"); 
  
  if (is_consumed)
  {
    token->type = Token_Type::As;
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

  // @todo João, não processa duas áspas simple em sequência que representam um àspa simples dentro da string
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

inline bool is_valid_ident_char(char c, bool quoted)
{
  return isalnum(c) || c == '_' || (quoted && c == ' ');
}

void try_parse_number(SQL_Parse_Context* parser, Token *token, bool *success)
{
  size_t i = 0;
  int32_t c = parser->peek_n_char(i);

  if (c == END_OF_SOURCE || parser->is_whitespace(c))
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }

  if (c == '0' && is_digit(parser->peek_n_char(i+1)))
  {
    token->type = Token_Type::None;
    *success = false;
    parser->error = true;
    return;
  }

  while (c != END_OF_SOURCE && !parser->is_whitespace(c) && !Is_Expression_Terminator(c))
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

// @note João, acho que copiar a `try_parse_string` e trocar o tipo de àspas já resolveria...
void try_parse_ident(SQL_Parse_Context* parser, Token *token, bool *success)
{
  const char double_quote = '"';
  bool is_quoted = false;
  bool saw_close_quote = false;
  size_t i = 0;
  int32_t c = parser->peek_n_char(i);

  if (c == double_quote)
  {
    is_quoted = true;
    i++;
    c = parser->peek_n_char(i);
  }
  
  // não pode começar com esse caracteres
  if (!is_quoted && (c == '_' || is_digit(c)))
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }
  
  while (c != END_OF_SOURCE && !Is_Expression_Terminator(c))
  {
    if (!is_quoted && parser->is_whitespace(c))
    {
      break;
    }

    // @todo João @wip ainda não lida com áspas duplas no meio de um ident com "quote"
    // @note João, vai bugar com ',' no meio de sequência 'quotadas'
    // @note João, buga também com "..." no meio das sequências 'quotadas'
    if (is_quoted && c == double_quote)
    {
      i++;
      c = parser->peek_n_char(i);
      saw_close_quote = true;
      break;
    }

    if (!is_valid_ident_char(c, is_quoted))
    {
      token->type = Token_Type::None;
      *success = false;
      return;
    }

    i++;
    c = parser->peek_n_char(i);
  }

  if (is_quoted && !saw_close_quote)
  {
    token->type = Token_Type::None;
    *success = false;
    return;
  }

  std::string ident_name = (is_quoted) ? 
    parser->source.substr(parser->index + 1, i - 2) :
    parser->source.substr(parser->index, i);
  
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

void SQL_Parse_Context::report_error(std::string error_message)
{
  this->error = true;
  this->error_message = error_message;
}
