#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "./ast_node.hpp"
#include "./ast_node_visitor.hpp"


struct Collector_Ast_Node_Visitor : Ast_Node_Visitor
{
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
    this->visit(*node.where);
    this->visit(*node.group_by);
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
  }

  void visit(Describe_Ast_Node &node)
  {
    idents.push_back(node.ident_name->ident_name);
  }
};
