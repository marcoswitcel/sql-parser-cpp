#include <iostream>
#include <string>
#include <algorithm>
#include <assert.h>

#include "../src/ast_node.hpp"
#include "../src/evaluate.cpp"

void test_run_like_pattern_on()
{
  assert(run_like_pattern_on("word", "word"));
  assert(run_like_pattern_on("test", "test"));
  assert(!run_like_pattern_on("test", "test2"));

  assert(run_like_pattern_on("word", "w_rd"));
  assert(run_like_pattern_on("ward", "w_rd"));
  assert(run_like_pattern_on("word", "w___"));
  assert(run_like_pattern_on("word", "____"));
  assert(!run_like_pattern_on("word", "_____"));
  assert(!run_like_pattern_on("words", "____"));

  assert(run_like_pattern_on("words", "w%"));
  // @todo João, ainda não suporta esses recursos...
  // assert(run_like_pattern_on("words", "%rds"));
  // assert(run_like_pattern_on("words", "%s"));
  // assert(run_like_pattern_on("words", "%r%"));
}

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
  test_run_like_pattern_on();
  std::cout << "test_run_like_pattern_on................................OK" << std::endl;

  std::cout << std::endl << "Fim testes" << std::endl;

  return EXIT_SUCCESS;
}
