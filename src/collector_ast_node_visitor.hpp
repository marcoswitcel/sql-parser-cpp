#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <assert.h>

#include "./ordered_map.hpp"
#include "./ast_node.hpp"
#include "./ast_node_visitor.hpp"

enum class Section_Type_Entry
{
  // o registro veio do select
  Select,
  Where,
  Group_By,
  Order_By,
  // o registro é o símbolo do comando describe
  Describe,
};

struct Ident_Entry
{
  Section_Type_Entry type;
  std::string ident;
  std::string as;
};

struct Collector_Ast_Node_Visitor : Ast_Node_Visitor
{
  // @note João, provavelmente vou precisar armazenar o ident taggeado com a informação de onde ele veio,
  // vou ter que considerar duplicidades, mas acho que num geral só preciso saber de cada ident uma vez por categoria
  // então pode ter o mesmo ident em categorias diferentes, mas não dentro da mesma categoria...
  // Não vi necessidade de fazer o mesmo pro resto, por hora não tem caso de uso, e se tiver, posso fazer com calma...
  std::vector<Ident_Entry> idents;
  Ordered_Map<std::string, Expression_Ast_Node*> alias;
  std::vector<std::string> strings;
  std::vector<int64_t> numbers;
  std::vector<std::string> froms;
  Section_Type_Entry current_type;

  void visit(Select_Ast_Node &node)
  {
    this->current_type = Section_Type_Entry::Select;

    for (auto field : node.fields)
    {
      // @todo João, aqui é um exemplo de lguar que precisaria ser ajustado, para
      // o dispatch funcionar para a classe mais específica precisaria escrever:
      // field->accept(*this); 
      this->visit(*field);
    }

    this->visit(*node.from);

    // campos opcionais

    this->current_type = Section_Type_Entry::Where;
    if (node.where) this->visit(*node.where);

    this->current_type = Section_Type_Entry::Group_By;
    if (node.group_by) this->visit(*node.group_by);
    
    this->current_type = Section_Type_Entry::Order_By;
    if (node.order_by) this->visit(*node.order_by);
  }

  void visit(From_Ast_Node &node)
  {
    froms.push_back(node.ident_name);
  }

  void visit(Where_Ast_Node &node)
  {
    this->visit(*node.conditions->left);
    this->visit(*node.conditions->right);
  }

  void visit(Group_By_Ast_Node &node)
  {
    for (auto &field : node.groups)
    {
      this->visit(*field);
    }
  }

  void visit(Order_By_Ast_Node &node)
  {
    for (auto &field : node.orders)
    {
      this->visit(*field);
    }
  }

  void visit(Expression_Ast_Node &node)
  {
    if (auto ident = Cast_If(Ident_Expression_Ast_Node, node))
    {
      Ident_Entry entry = { .type = this->current_type, .ident = ident->ident_name, .as = ident->as };
      idents.push_back(entry);
    }
    else if (auto number = Cast_If(Number_Literal_Expression_Ast_Node, node))
    {
      numbers.push_back(number->value);
    }
    else if (auto string = Cast_If(String_Literal_Expression_Ast_Node, node))
    {
      
      strings.push_back(string->value);
    }
    else if (auto binary_expression = Cast_If(Binary_Expression_Ast_Node, node))
    {
      this->visit(*binary_expression->left);
      this->visit(*binary_expression->right);
    }
    else if (auto function_call = Cast_If(Function_Call_Expression_Ast_Node, node))
    {
      for (auto argument : function_call->argument_list)
      {
        this->visit(*argument);
      }
    }
    else if (auto ordering = Cast_If(Ordering_Expression_Ast_Node, node))
    {
      this->visit(*ordering->expr);
    }
    else
    {
      // @note se cair aqui é porque foi esquecido de lidar com alguma sub-expressão
      assert(false);
    }

    // armazena todos os alias
    if (this->current_type == Section_Type_Entry::Select && !node.as.empty())
    {
      alias.put(node.as, &node);
    }

    // @todo João, considerar como incluir o renome de campos como idents aqui.. no futuro vou precisar deles...
    // expressões complexas deverão ser suportadas para fim de referência no where
    // Exemple: select Id + 2 as Ids where Ids > 2;
  }

  void visit(Describe_Ast_Node &node)
  {
    this->current_type = Section_Type_Entry::Describe;
    
    Ident_Entry entry = { .type = this->current_type, .ident = node.ident_name->ident_name, .as = "" };
    idents.push_back(entry);
  }
};
