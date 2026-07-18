#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <assert.h>

#include "./ast_node.hpp"
#include "./ast_node_visitor.hpp"


struct Collector_Ast_Node_Visitor : Ast_Node_Visitor
{
  // @todo João, provavelmente vou precisar armazenar o ident taggeado com a informação de onde ele veio,
  // vou ter que considerar duplicidades, mas acho que num geral só preciso saber de cada ident uma vez por categoria
  // então pode ter o mesmo ident em categorias diferentes, mas não dentro da mesma categoria...
  // Não vi necessidade de fazer o mesmo pro resto, por hora não tem caso de uso, e se tiver, posso fazer com calma...
  std::vector<std::string> idents; 
  std::vector<std::string> strings;
  std::vector<int64_t> numbers;
  std::vector<std::string> froms;

  void visit(Select_Ast_Node &node)
  {
    for (auto field : node.fields)
    {
      this->visit(*field);
    }

    this->visit(*node.from);
    // campos opcionais
    if (node.where) this->visit(*node.where);
    if (node.group_by) this->visit(*node.group_by);
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
      idents.push_back(ident->ident_name);
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
  }

  void visit(Describe_Ast_Node &node)
  {
    idents.push_back(node.ident_name->ident_name);
  }
};
