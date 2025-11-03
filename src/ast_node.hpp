#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <assert.h>
#include <regex>
#include <memory>

#include "./utils.cpp"
#include "./trace.hpp"

enum class Inferred_Type
{
  Not_Inferred,
  String,
  Number,
};

enum class Ast_Node_Type
{
  None = 0,
  Select_Ast_Node = 1 << 0,
  From_Ast_Node = 1 << 1,
  Where_Ast_Node = 1 << 2,
  Describe_Ast_Node = 1 << 3,
  Expression_Node = 1 << 4, // categoria
  String_Literal_Expression_Ast_Node = Expression_Node | (1 << 5),
  Number_Literal_Expression_Ast_Node = Expression_Node | (1 << 6),
  Ident_Expression_Ast_Node          = Expression_Node | (1 << 7),
  Function_Call_Expression_Ast_Node  = Expression_Node | (1 << 8),
  Binary_Expression_Node = Expression_Node | (1 << 9), // sub-categoria
};

Ast_Node_Type operator&(Ast_Node_Type a, Ast_Node_Type b)
{
  return static_cast<Ast_Node_Type>(static_cast<uint64_t>(a) & static_cast<uint64_t>(b));
}

Ast_Node_Type operator|(Ast_Node_Type a, Ast_Node_Type b)
{
  return static_cast<Ast_Node_Type>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
}

inline bool ast_sub_type_of(Ast_Node_Type maybe_sub_type, Ast_Node_Type type) noexcept
{
  return (maybe_sub_type & type) == type;
}

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

  virtual ~Ast_Node()
  {
    assert(this->type != Ast_Node_Type::None);
    // @note apenas para depuração
    // std::cout << "destruído" << this->serial_number << std::endl;
  }

  virtual std::string to_string() = 0;

  virtual std::string to_expression()
  {
    return "[expression]";
  }
};

uint64_t Ast_Node::serial_counter = 0;

struct Expression_Ast_Node: Ast_Node
{
  virtual std::string to_string() = 0;

  // @note João, incluído aqui um atributo para conter informações sobre um possível
  // `as` keyword. Acredito que deixaria mais compacto e de fácil acesso. Avaliar.
  std::string as;
  Inferred_Type inferred_type = Inferred_Type::Not_Inferred;

  virtual Inferred_Type infer_type()
  {
    return this->inferred_type;
  }
};

struct Ident_Expression_Ast_Node: Expression_Ast_Node
{
  std::string ident_name;
  
  Ident_Expression_Ast_Node()
  {
    this->type = Ast_Node_Type::Ident_Expression_Ast_Node;
    // @note João, por hora todos os Ident's são Strings
    this->inferred_type = Inferred_Type::String;
  }

  std::string to_string() override
  {
    std::string desc = "Ident_Expression_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", ident_name: \"" + this->ident_name + "\"";
    if (!this->as.empty()) desc += ", as: \"" + this->as +  "\"";
    desc += " }";

    return desc;
  }

  std::string to_expression() override
  {
    // @todo João, complementar essa validação para incluir qualquer caractere especial
    if (contains(this->ident_name, ' '))
    {
      return "\"" + this->ident_name + "\"";
    }

    return this->ident_name;
  }
};

struct Function_Call_Expression_Ast_Node: Expression_Ast_Node
{
  std::string name;
  std::vector<Expression_Ast_Node*> argument_list;
  
  Function_Call_Expression_Ast_Node()
  {
    this->type = Ast_Node_Type::Function_Call_Expression_Ast_Node;
  }

  std::string to_string() override
  {
    // @todo João, falta por a lista de argumentos aqui
    std::string desc = "Function_Call_Expression_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", name: \"" + this->name + "\"";
    if (!this->as.empty()) desc += ", as: \"" + this->as +  "\"";
    desc += " }";

    return desc;
  }

  std::string to_expression() override
  {
    // @todo João, ajustar para considerar lista de argumentos
    std::string expr = this->name + "(";
    for (size_t i = 0; i < this->argument_list.size(); i++)
    {
      if (i != 0) expr += ", ";
      expr += this->argument_list.at(i)->to_expression();
    }
    expr += ")";
    return expr;
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
    this->inferred_type = Inferred_Type::String;
  }

  std::string to_string() override
  {
    std::string desc = "String_Literal_Expression_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", value: \"" + this->value +  "\" }";

    return desc;
  }

  std::string to_expression() override
  {
    static std::regex quote("'");
    return "'" + std::regex_replace(this->value, quote, "''") + "'";
  }
};

struct Number_Literal_Expression_Ast_Node: Expression_Ast_Node
{
  int64_t value;
  
  Number_Literal_Expression_Ast_Node()
  {
    this->type = Ast_Node_Type::Number_Literal_Expression_Ast_Node;
  }

  std::string to_string() override
  {
    std::string desc = "Number_Literal_Expression_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", value: " + std::to_string(this->value) +  " }";

    return desc;
  }

  std::string to_expression() override
  {
    return std::to_string(this->value);
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

  std::string to_expression() override
  {
    return this->left->to_expression() + " || " + this->right->to_expression();
  }
};

struct Where_Ast_Node: Ast_Node
{
  std::unique_ptr<Binary_Expression_Ast_Node> conditions;
  
  Where_Ast_Node()
  {
    this->type = Ast_Node_Type::Where_Ast_Node;
    // std::cout << "construído ident" << this->serial_number << std::endl;
  }

  std::string to_string() override
  {
    std::string desc = "Where_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);

    desc += ", conditions: ";
    {
      if (this->conditions.get())
      {
        desc += this->conditions->to_string();
      }
    }
    desc += " }";

    return desc;
  }
};

struct Select_Ast_Node: Ast_Node
{
  std::vector<std::shared_ptr<Expression_Ast_Node>> fields;
  std::shared_ptr<From_Ast_Node> from;
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

struct Describe_Ast_Node: Ast_Node
{
  std::string ident_name;
  
  Describe_Ast_Node()
  {
    this->type = Ast_Node_Type::Describe_Ast_Node;
  }

  std::string to_string() override
  {
    std::string desc = "Describe_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", ident_name: \"" + this->ident_name +  "\" }";

    return desc;
  }
};
