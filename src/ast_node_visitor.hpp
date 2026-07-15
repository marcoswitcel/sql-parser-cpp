#pragma once


// declarando antecipadamento todos os tipos de Ast_Node
struct Select_Ast_Node;
struct From_Ast_Node;
struct Where_Ast_Node;
struct Group_By_Ast_Node;
struct Order_By_Ast_Node;
struct Expression_Ast_Node;
struct Describe_Ast_Node;

struct Ast_Node_Visitor
{
  virtual ~Ast_Node_Visitor() = default;

  virtual void visit(Select_Ast_Node &node) = 0;
  virtual void visit(From_Ast_Node &node) = 0;
  virtual void visit(Where_Ast_Node &node) = 0;
  virtual void visit(Group_By_Ast_Node &node) = 0;
  virtual void visit(Order_By_Ast_Node &node) = 0;
  // @todo João, adicionar Ordering_Expression_Ast_node ou implementar no visit abaixo
  virtual void visit(Expression_Ast_Node &node) = 0;
  virtual void visit(Describe_Ast_Node &node) = 0;
};
