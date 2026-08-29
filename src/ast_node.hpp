#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdint>
#include <assert.h>
#include <memory>

#include "./ast_node_visitor.hpp"
#include "./builtin_function_definition.hpp"
#include "./token.hpp"
#include "./utils.cpp"
#include "./trace.hpp"

#define Ast_Node_To_String_Start(TYPE) std::ostringstream private_builder; private_builder << #TYPE " { serial: " << this->serial_number;
#define Ast_Node_To_String_As_Field() if (!this->as.empty()) private_builder << ", as: \"" << this->as <<  "\"";
#define Ast_Node_To_String_Add_String_Field(FIELD_NAME, VALUE) private_builder << ", " #FIELD_NAME ": \"" << (VALUE) << "\"";
#define Ast_Node_To_String_Add_Field(FIELD_NAME, VALUE) private_builder << ", " #FIELD_NAME ": " << (VALUE);

#define Ast_Node_To_String_List_Field(FIELD_NAME, EXPRESSION) \
  private_builder << ", " #FIELD_NAME ": [";                  \
  for (size_t i = 0; i < (this->FIELD_NAME).size(); i++)      \
  {                                                           \
    if (i > 0)                                                \
    {                                                         \
      private_builder << ", ";                                \
    }                                                         \
    auto &it = (this->FIELD_NAME).at(i);                      \
    private_builder << (EXPRESSION);                          \
  }                                                           \
  private_builder << "]";

#define Ast_Node_To_String_End() private_builder << " }";
#define Ast_Node_To_String_Get_Result() (private_builder.str());

enum class Inferred_Type
{
  Not_Inferred,
  String,
  Number,
};

enum class Ast_Node_Type
{
  None                = 0,
  Select_Ast_Node     = 1 << 0,
  From_Ast_Node       = 1 << 1,
  Where_Ast_Node      = 1 << 2,
  Group_By_Ast_Node   = 1 << 3,
  Order_By_Ast_Node   = 1 << 4,
  Describe_Ast_Node   = 1 << 5,
  Expression_Ast_Node = 1 << 6, // categoria
  Ordering_Expression_Ast_Node       = Expression_Ast_Node | (1 << 7),
  String_Literal_Expression_Ast_Node = Expression_Ast_Node | (1 << 8),
  Number_Literal_Expression_Ast_Node = Expression_Ast_Node | (1 << 9),
  Ident_Expression_Ast_Node          = Expression_Ast_Node | (1 << 10),
  Function_Call_Expression_Ast_Node  = Expression_Ast_Node | (1 << 11),
  Binary_Expression_Ast_Node         = Expression_Ast_Node | (1 << 12), // sub-categoria
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

  /**
   * @brief retorna um representação textual útil para logs e depuração em geral
   * @note João, considerar implementar alguma função por template ou macro para auxiliar no print de depuração padronizado
   * 
   * @return std::string 
   */
  virtual std::string to_string() = 0;

  /**
   * @brief permite o uso de uma instância de `Ast_Node_Visitor`
   * 
   * @param visitor 
   */
  virtual void accept(Ast_Node_Visitor &visitor) = 0;

  /**
   * @brief retorna a representação textual SQL canônica
   * 
   * @return std::string 
   */
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

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
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
    Ast_Node_To_String_Start(Ident_Expression_Ast_Node);
    Ast_Node_To_String_Add_String_Field(ident_name, this->ident_name);
    Ast_Node_To_String_End();

    return Ast_Node_To_String_Get_Result();
  }

  std::string to_expression() override
  {
    std::string result = symbol_as_expression(this->ident_name);
    
    if (this->as.size() > 0)
    {
      // @note João, mover isso pra dentro da classe Expression_Ast_Node
      result += " As " + this->as;
    }

    return result;
  }

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }
};

struct Function_Call_Expression_Ast_Node: Expression_Ast_Node
{
  std::string name;
  Builtin_Function_Names tagged_name;
  std::vector<Expression_Ast_Node*> argument_list;
  
  Function_Call_Expression_Ast_Node()
  {
    this->type = Ast_Node_Type::Function_Call_Expression_Ast_Node;
  }

  std::string to_string() override
  {
    // @todo João, falta por a lista de argumentos aqui
    Ast_Node_To_String_Start(Function_Call_Expression_Ast_Node);
    Ast_Node_To_String_As_Field();
    Ast_Node_To_String_Add_String_Field(name, this->name);
    Ast_Node_To_String_Add_Field(tagged_name, ::to_string(this->tagged_name));
    Ast_Node_To_String_List_Field(argument_list, it->to_string());
    Ast_Node_To_String_End();

    return Ast_Node_To_String_Get_Result();
  }

  std::string to_expression() override
  {
    std::string expr = this->name + "(";
    for (size_t i = 0; i < this->argument_list.size(); i++)
    {
      if (i != 0) expr += ", ";
      expr += this->argument_list.at(i)->to_expression();
    }
    expr += ")";
    return expr;
  }

  Inferred_Type infer_type() override
  {
    if (this->inferred_type == Inferred_Type::Not_Inferred)
    {
      if (this->tagged_name == Builtin_Function_Names::CURRENT_DATE)
      {
        this->inferred_type = Inferred_Type::String;
      }
      else if (this->tagged_name == Builtin_Function_Names::LOWER)
      {
        this->inferred_type = Inferred_Type::String;
      }
      else if (this->tagged_name == Builtin_Function_Names::UPPER)
      {
        this->inferred_type = Inferred_Type::String;
      }
      else if (this->tagged_name == Builtin_Function_Names::SUBSTRING)
      {
        this->inferred_type = Inferred_Type::String;
      }
      else if (this->tagged_name == Builtin_Function_Names::COALESCE)
      {
        // @note João, avaliar se deve retornar tipos diferentes (quando tivermos suporte)
        this->inferred_type = Inferred_Type::String;
      }
      else if (this->tagged_name == Builtin_Function_Names::TO_NUMBER)
      {
        this->inferred_type = Inferred_Type::Number;
      }
      else if (this->tagged_name == Builtin_Function_Names::MAX)
      {
        this->inferred_type = Inferred_Type::Number;
      }
      else if (this->tagged_name == Builtin_Function_Names::MIN)
      {
        this->inferred_type = Inferred_Type::Number;
      }
      else if (this->tagged_name == Builtin_Function_Names::COUNT)
      {
        this->inferred_type = Inferred_Type::Number;
      }
      else if (this->tagged_name == Builtin_Function_Names::SUM)
      {
        this->inferred_type = Inferred_Type::Number;
      }
      else if (this->tagged_name == Builtin_Function_Names::AVG)
      {
        this->inferred_type = Inferred_Type::Number;
      }
      else if (this->tagged_name == Builtin_Function_Names::FIRST_VALUE)
      {
        // @note João, avaliar se deve retornar tipos diferentes
        this->inferred_type = Inferred_Type::String;
      }
    }

    // @note João, não é o melhor, mas por hora é isso...
    for (auto &arg : this->argument_list)
    {
      arg->infer_type();
    }

    return this->inferred_type;
  }

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }
};

struct From_Ast_Node: Ast_Node
{
  std::string ident_name;
  
  From_Ast_Node()
  {
    this->type = Ast_Node_Type::From_Ast_Node;
  }

  std::string to_string() override
  {
    Ast_Node_To_String_Start(From_Ast_Node);
    Ast_Node_To_String_Add_String_Field(ident_name, this->ident_name);
    Ast_Node_To_String_End();

    return Ast_Node_To_String_Get_Result();
  }

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }

  std::string to_expression() override
  {
    return symbol_as_expression(this->ident_name);
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
    Ast_Node_To_String_Start(String_Literal_Expression_Ast_Node);
    Ast_Node_To_String_Add_String_Field(value, this->value);
    Ast_Node_To_String_End();

    return Ast_Node_To_String_Get_Result();
  }

  std::string to_expression() override
  {
    return "'" + replace_char_with(this->value, '\'', "''") + "'";
  }

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }
};

struct Number_Literal_Expression_Ast_Node: Expression_Ast_Node
{
  int64_t value;
  
  Number_Literal_Expression_Ast_Node()
  {
    this->type = Ast_Node_Type::Number_Literal_Expression_Ast_Node;
    this->inferred_type = Inferred_Type::Number;
  }

  std::string to_string() override
  {
    Ast_Node_To_String_Start(Number_Literal_Expression_Ast_Node);
    Ast_Node_To_String_Add_Field(value, this->value);
    Ast_Node_To_String_End();

    return Ast_Node_To_String_Get_Result();
  }

  std::string to_expression() override
  {
    return std::to_string(this->value);
  }

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }
};

struct Binary_Expression_Ast_Node: Expression_Ast_Node
{
  // @todo João, por hora é uma string mas deveria ser um enum no futuro
  std::string op;
  std::unique_ptr<Expression_Ast_Node> left;
  std::unique_ptr<Expression_Ast_Node> right;

  Binary_Expression_Ast_Node()
  {
    this->type = Ast_Node_Type::Binary_Expression_Ast_Node;
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
    std::string op = this->op;

    if (!op.empty())
    {
      op[0] = std::toupper(op[0]);
    }

    return this->left->to_expression() + " " + op + " " + this->right->to_expression();
  }

  Inferred_Type infer_type() override
  {
    if (this->op == "concat")
    {
      this->inferred_type = Inferred_Type::String;
    }

    return this->inferred_type;
  }

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }
};

struct Ordering_Expression_Ast_Node: Expression_Ast_Node
{
  std::unique_ptr<Expression_Ast_Node> expr;
  Token_Type dir = Token_Type::None;
 
  Ordering_Expression_Ast_Node()
  {
    this->type = Ast_Node_Type::Ordering_Expression_Ast_Node;
  }

  std::string to_string() override
  {
    std::string desc = "Ordering_Expression_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", expr: " + ((this->expr.get()) ? this->expr->to_string() : "NULL");
    desc += " }";

    return desc;
  }

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }

  virtual std::string to_expression()
  {
    std::string result = this->expr->to_expression();
    
    if (this->dir != Token_Type::None)
    {
      result += " " + get_description(this->dir);
    }

    return result;
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

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }

  virtual std::string to_expression()
  {
    std::string result = "Where ";

    result += this->conditions->to_expression();

    return result;
  }
};

struct Group_By_Ast_Node: Ast_Node
{
  std::vector<std::unique_ptr<Expression_Ast_Node>> groups;
  
  Group_By_Ast_Node()
  {
    this->type = Ast_Node_Type::Group_By_Ast_Node;
  }

  std::string to_string() override
  {
    std::string desc = "Group_By_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", groups: [] }"; // @todo João, terminar aqui...

    return desc;
  }

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }

  virtual std::string to_expression()
  {
    std::string result = "Group By";

    for (size_t i = 0; i < this->groups.size(); i++)
    {
      auto &group = this->groups.at(i);
      result += " " + group->to_expression();
      if (i + 1 != this->groups.size())
      {
        result += ",";
      }
    }

    return result;
  }
};

struct Order_By_Ast_Node: Ast_Node
{
  std::vector<std::unique_ptr<Ordering_Expression_Ast_Node>> orders;
  
  Order_By_Ast_Node()
  {
    this->type = Ast_Node_Type::Order_By_Ast_Node;
  }

  std::string to_string() override
  {
    std::string desc = "Order_By_Ast_Node { serial: ";
    desc += std::to_string(this->serial_number);
    desc += ", orders: [] }"; // @todo João, terminar aqui...

    return desc;
  }

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }

  virtual std::string to_expression()
  {
    std::string result = "Order By";

    for (size_t i = 0; i < this->orders.size(); i++)
    {
      auto &order = this->orders.at(i);
      result += " " + order->to_expression();
      if (i + 1 != this->orders.size())
      {
        result += ",";
      }
    }

    return result;
  }
};


struct Select_Ast_Node: Ast_Node
{
  std::vector<std::shared_ptr<Expression_Ast_Node>> fields;
  std::shared_ptr<From_Ast_Node> from;
  /**
   * @brief define se o select possuia uma cláusula where e as expressões associadas
   * Campo opcional
   */
  std::unique_ptr<Where_Ast_Node> where;
  /**
   * @brief define se o select possuía uma regra de agrupamento associada
   * Campo opcional
   */
  std::unique_ptr<Group_By_Ast_Node> group_by;
  /**
   * @brief define se o select possuía uma regra de ordenação associada
   * Campo opcional
   */
  std::unique_ptr<Order_By_Ast_Node> order_by;

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

  std::string to_expression() override
  {
    std::string result = "";
    result += "Select ";
    for (size_t i = 0; i < this->fields.size(); i++)
    {
      if (i > 0)
      {
        result += ", ";
      }
      auto field = this->fields.at(i).get();
      result += field->to_expression();
    }
    result += " From ";
    result += this->from->to_expression();

    if (this->where)
    {
      result += " ";
      result += this->where->to_expression();
    }

    if (this->group_by)
    {
      result += " ";
      result += this->group_by->to_expression();
    }

    if (this->order_by)
    {
      result += " ";
      result += this->order_by->to_expression();
    }

    return result;
  }  

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }
};

struct Describe_Ast_Node: Ast_Node
{
  std::unique_ptr<Ident_Expression_Ast_Node> ident_name;
  
  Describe_Ast_Node()
  {
    this->type = Ast_Node_Type::Describe_Ast_Node;
  }

  std::string to_string() override
  {
    Ast_Node_To_String_Start(Describe_Ast_Node);
    Ast_Node_To_String_Add_String_Field(ident_name, this->ident_name->ident_name);
    Ast_Node_To_String_End();

    return Ast_Node_To_String_Get_Result();
  }

  std::string to_expression() override
  {
    return "Describe " + this->ident_name->to_expression();
  }

  void accept(Ast_Node_Visitor &visitor) override
  {
    visitor.visit(*this);
  }
};

/**
 * @brief faz o cast para o tipo específico se o campo type bater com o tipo solicitado, senão retorna null
 * @note João, considerar fazer a versão com função genérica e check baseado no atributo estático
 * 
 */
#define Cast_If(Type, Ast_Node_Instance) ((Ast_Node_Instance).type == Ast_Node_Type::Type ? static_cast<Type*>(&(Ast_Node_Instance)) : NULL)
