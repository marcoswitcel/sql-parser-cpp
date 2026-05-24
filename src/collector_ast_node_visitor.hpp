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
    if (node.type == Ast_Node_Type::Ident_Expression_Ast_Node)
    {
      auto ident = static_cast<Ident_Expression_Ast_Node*>(&node);
      idents.push_back(ident->ident_name);
    }
    else if (node.type == Ast_Node_Type::Number_Literal_Expression_Ast_Node)
    {
      auto number = static_cast<Number_Literal_Expression_Ast_Node*>(&node);
      numbers.push_back(number->value);
    }
    else if (node.type == Ast_Node_Type::String_Literal_Expression_Ast_Node)
    {
      auto string = static_cast<String_Literal_Expression_Ast_Node*>(&node);
      strings.push_back(string->value);
    }
    else if (node.type == Ast_Node_Type::Binary_Expression_Ast_Node)
    {
      auto binary_expression = static_cast<Binary_Expression_Ast_Node*>(&node);
      this->visit(*binary_expression->left);
      this->visit(*binary_expression->right);
    }
    else if (node.type == Ast_Node_Type::Function_Call_Expression_Ast_Node)
    {
      auto function_call = static_cast<Function_Call_Expression_Ast_Node*>(&node);

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
