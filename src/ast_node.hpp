#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

struct Ast_Node
{
  static uint64_t serial_counter;

  uint64_t serial_number = 0;

  Ast_Node()
  {
    this->serial_number = ++Ast_Node::serial_counter;

    // @note apenas para depuração
    // std::cout << "construído" << this->serial_number << std::endl;
  }

  ~Ast_Node()
  {
    // @note apenas para depuração
    // std::cout << "destruído" << this->serial_number << std::endl;
  }

  virtual std::string to_string()
  {
    std::string desc = "Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += " }";

    return desc;
  }
};

uint64_t Ast_Node::serial_counter = 0;

struct Ident_Ast_Node: Ast_Node
{
  std::string ident_name;
  
  Ident_Ast_Node()
  {
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

struct Select_Ast_Node: Ast_Node
{
  std::vector<std::shared_ptr<Ident_Ast_Node>> fields;
  std::shared_ptr<Ident_Ast_Node> from;

  std::string to_string() override
  {
    std::string desc = "Select_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", fields: [ ";
    for (auto &field : fields)
    {
      desc += field.get()->to_string();
    }
    desc += " ]";

    desc += " }";

    return desc;
  }
};
