#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <assert.h>
#include <memory>

#include "./trace.hpp"


enum class Ast_Node_Type
{
  None,
  Ident_Expression_Ast_Node,
  String_Literal_Expression_Ast_Node,
  Select_Ast_Node,
  From_Ast_Node,
  Where_Ast_Node,
  Binary_Expression_Node,
};

struct Ast_Node
{
  static uint64_t serial_counter;

  uint64_t serial_number = 0;
  Ast_Node_Type type;

  Ast_Node()
  {
    this->serial_number = ++Ast_Node::serial_counter;
    this->type = Ast_Node_Type::None;

    // @note apenas para depuração
    // std::cout << "construído" << this->serial_number << std::endl;
  }

  ~Ast_Node()
  {
    assert(this->type != Ast_Node_Type::None);
    // @note apenas para depuração
    // std::cout << "destruído" << this->serial_number << std::endl;
  }

  virtual std::string to_string() = 0;
};

uint64_t Ast_Node::serial_counter = 0;

struct Expression_Ast_Node: Ast_Node
{
  virtual std::string to_string() = 0;
};

// @todo João, considerar renomear para Ident_Expression_Ast_Node e
// ajustar o enum para ser usado como flags para os subgrupos?
struct Ident_Expression_Ast_Node: Expression_Ast_Node
{
  std::string ident_name;
  
  Ident_Expression_Ast_Node()
  {
    this->type = Ast_Node_Type::Ident_Expression_Ast_Node;
    // std::cout << "construído ident" << this->serial_number << std::endl;
  }

  std::string to_string() override
  {
    std::string desc = "Ident_Expression_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", ident_name: \"" + this->ident_name +  "\" }";

    return desc;
  }
};

struct From_Ast_Node: Ast_Node
{
  std::string ident_name;
  
  From_Ast_Node()
  {
    this->type = Ast_Node_Type::From_Ast_Node;
    // std::cout << "construído ident" << this->serial_number << std::endl;
  }

  std::string to_string() override
  {
    std::string desc = "From_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", ident_name: \"" + this->ident_name +  "\" }";

    return desc;
  }
};

struct Literal_Expression_Ast_Node: Expression_Ast_Node
{
  virtual std::string to_string() = 0;
};

struct String_Literal_Expression_Ast_Node: Expression_Ast_Node
{
  std::string value;
  
  String_Literal_Expression_Ast_Node()
  {
    this->type = Ast_Node_Type::String_Literal_Expression_Ast_Node;
  }

  std::string to_string() override
  {
    std::string desc = "String_Literal_Expression_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", value: \"" + this->value +  "\" }";

    return desc;
  }
};

struct Binary_Expression_Ast_Node: Expression_Ast_Node
{
  // @note João, por hora string mas pode ser um enum no futuro?
  std::string op;
  std::unique_ptr<Expression_Ast_Node> left;
  std::unique_ptr<Expression_Ast_Node> right;

  Binary_Expression_Ast_Node()
  {
    this->type = Ast_Node_Type::Binary_Expression_Node;
  }

  std::string to_string() override
  {
    std::string desc = "Binary_Expression_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", op: '" + this->op + "'";
    desc += ", left: " + ((this->left.get()) ? this->left->to_string() : "NULL");
    desc += ", right: " + ((this->right.get()) ? this->right->to_string() : "NULL");
    desc += " }";

    return desc;
  }
};

struct Where_Ast_Node: Ast_Node
{
  // @todo João, avaliar se não deveria ser um ponteiro apenas ao invés de uma lista de expressões
  std::vector<std::unique_ptr<Binary_Expression_Ast_Node>> conditions;
  
  Where_Ast_Node()
  {
    this->type = Ast_Node_Type::Where_Ast_Node;
    // std::cout << "construído ident" << this->serial_number << std::endl;
  }

  std::string to_string() override
  {
    std::string desc = "Where_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);

    desc += ", conditions: [ ";
    for (size_t i = 0; i < this->conditions.size(); i++)
    {
      if (i > 0)
      {
        desc += " , ";
      }
      auto &condition = this->conditions.at(i);
      desc += condition.get()->to_string();
    }
    desc += " ] ";
    desc += "}";

    return desc;
  }
};

struct Select_Ast_Node: Ast_Node
{
  std::vector<std::shared_ptr<Ident_Expression_Ast_Node>> fields;
  std::shared_ptr<From_Ast_Node> from;
  // @todo joão, implementar where
  std::unique_ptr<Where_Ast_Node> where;

  Select_Ast_Node()
  {
    this->type = Ast_Node_Type::Select_Ast_Node;
  }

  std::string to_string() override
  {
    std::string desc = "Select_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", fields: [ ";
    for (size_t i = 0; i < fields.size(); i++)
    {
      if (i > 0)
      {
        desc += " , ";
      }
      auto &field = fields.at(i);
      desc += field.get()->to_string();
    }
    desc += " ], ";
    desc += "from: ";
    desc += from.get()->to_string();
    desc += ", where: ";
    desc += (this->where.get()) ? this->where->to_string() : "{}";
    desc += " }";

    return desc;
  }
};
