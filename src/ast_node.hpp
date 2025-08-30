#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <assert.h>
#include <memory>

enum class Ast_Node_Type
{
  None,
  Ident_Ast_Node,
  Select_Ast_Node,
  From_Ast_Node,
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

struct Ident_Ast_Node: Ast_Node
{
  std::string ident_name;
  
  Ident_Ast_Node()
  {
    this->type = Ast_Node_Type::Ident_Ast_Node;
    // std::cout << "construído ident" << this->serial_number << std::endl;
  }

  std::string to_string() override
  {
    std::string desc = "Ident_Ast_Node { serial: ";
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

struct Expression_Ast_Node: Ast_Node
{

};

struct Literal_Expression_Ast_Node: Expression_Ast_Node
{

};

struct Select_Ast_Node: Ast_Node
{
  std::vector<std::shared_ptr<Ident_Ast_Node>> fields;
  std::shared_ptr<From_Ast_Node> from;
  // @todo joão, implementar where
  std::vector<std::unique_ptr<Expression_Ast_Node>> where;

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
    desc += " }";

    return desc;
  }
};
