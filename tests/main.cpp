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
  assert(run_like_pattern_on("words", "%rds"));
  assert(run_like_pattern_on("words", "%s"));
  assert(run_like_pattern_on("words", "%r%"));
  assert(run_like_pattern_on("words", "%_r_%"));
  assert(run_like_pattern_on("words", "%w%s%"));
  assert(run_like_pattern_on("words", "%w%ds%"));
  assert(!run_like_pattern_on("word", "%w%d_%"));
  assert(run_like_pattern_on("A small text sample", "%"));
  assert(run_like_pattern_on("A small text sample", "A small%"));
  assert(run_like_pattern_on("A small text sample", "%small%"));
  assert(run_like_pattern_on("A small text sample", "%sm_ll%"));
  assert(run_like_pattern_on("Moon", "%____%"));
  assert(!run_like_pattern_on("Car", "%____%"));

  // case insensitive tests
  assert(run_like_pattern_on("Word The Other Word", "%The%"));
  assert(run_like_pattern_on("Word the Other Word", "%The%"));
  assert(run_like_pattern_on("Word The Other Word", "%the%"));
  assert(run_like_pattern_on("Word the Other Word", "%the%"));
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
