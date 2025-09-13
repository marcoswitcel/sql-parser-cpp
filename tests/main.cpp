#include <iostream>
#include <string>
#include <algorithm>
#include <assert.h>

#include "../src/ast_node.hpp"

void test_ast_node_type()
{
  assert(ast_sub_type_of(Ast_Node_Type::Binary_Expression_Node, Ast_Node_Type::Expression_Node));
  assert(ast_sub_type_of(Ast_Node_Type::Ident_Expression_Ast_Node, Ast_Node_Type::Expression_Node));
  assert(ast_sub_type_of(Ast_Node_Type::String_Literal_Expression_Ast_Node, Ast_Node_Type::Expression_Node));
}

int main()
{
  std::cout << "Iniciando testes" << std::endl << std::endl;

  test_ast_node_type();
  std::cout << "test_ast_node_type......................................OK" << std::endl;

  std::cout << std::endl << "Fim testes" << std::endl;

  return EXIT_SUCCESS;
}
