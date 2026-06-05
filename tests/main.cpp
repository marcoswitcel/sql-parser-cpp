#include <iostream>
#include <string>
#include <algorithm>
#include <assert.h>

#include "../src/ordered_map.hpp"
#include "../src/aggregator.cpp"
#include "../src/utils.cpp"
#include "../src/ast_node.hpp"
#include "../src/sql-parse-context.cpp"
#include "../src/evaluate.cpp"
#include "./csv_data_set.cpp"

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
  assert(ast_sub_type_of(Ast_Node_Type::Binary_Expression_Ast_Node, Ast_Node_Type::Expression_Ast_Node));
  assert(ast_sub_type_of(Ast_Node_Type::Ident_Expression_Ast_Node, Ast_Node_Type::Expression_Ast_Node));
  assert(ast_sub_type_of(Ast_Node_Type::String_Literal_Expression_Ast_Node, Ast_Node_Type::Expression_Ast_Node));
  assert(ast_sub_type_of(Ast_Node_Type::Number_Literal_Expression_Ast_Node, Ast_Node_Type::Expression_Ast_Node));
  assert(ast_sub_type_of(Ast_Node_Type::Function_Call_Expression_Ast_Node, Ast_Node_Type::Expression_Ast_Node));
}

void test_parse_describe_01()
{
  std::string sql = "Describe Iris";
  SQL_Parse_Context parser(sql);

  Ast_Node* node = parser.eat_node();

  assert(node != NULL);
  assert(node->type == Ast_Node_Type::Describe_Ast_Node);

  Describe_Ast_Node* describe = static_cast<Describe_Ast_Node*>(node);

  assert(describe->ident_name->ident_name == "Iris");
  assert(ignore_case_equals(describe->to_expression(), sql));
}

void test_parse_describe_02()
{
  std::string sql = "Describe \"Iris 2\"";
  SQL_Parse_Context parser(sql);

  Ast_Node* node = parser.eat_node();

  assert(node != NULL);
  assert(node->type == Ast_Node_Type::Describe_Ast_Node);

  Describe_Ast_Node* describe = static_cast<Describe_Ast_Node*>(node);

  assert(describe->ident_name->ident_name == "Iris 2");
  assert(ignore_case_equals(describe->to_expression(), sql));
}

void test_parse_select_01()
{
  std::string sql = "Select *, teste as renomeado From csv_filename Where a = 2 and b like '%b%'";
  SQL_Parse_Context parser(sql);

  Ast_Node* node = parser.eat_node();

  assert(node != NULL);
  assert(node->type == Ast_Node_Type::Select_Ast_Node);

  Select_Ast_Node* select = static_cast<Select_Ast_Node*>(node);

  assert(select->fields.size() == 2);
  
  assert(select->fields.at(0).get()->type == Ast_Node_Type::Ident_Expression_Ast_Node);
  {
    auto ident = static_cast<Ident_Expression_Ast_Node*>(select->fields.at(0).get());
    assert(ident->ident_name == "*");
    assert(ident->as.size() == 0);
  }

  assert(select->fields.at(1).get()->type == Ast_Node_Type::Ident_Expression_Ast_Node);
  {
    auto ident = static_cast<Ident_Expression_Ast_Node*>(select->fields.at(1).get());
    assert(ident->ident_name == "teste");
    assert(ident->as == "renomeado");
  }
  
  assert(select->from->ident_name == "csv_filename");

  assert(select->where->conditions.get() != NULL);

  auto conditions = static_cast<Binary_Expression_Ast_Node*>(select->where->conditions.get());
  assert(conditions->op == "and");
  assert(conditions->left.get()->type == Ast_Node_Type::Binary_Expression_Ast_Node);
  assert(conditions->right.get()->type == Ast_Node_Type::Binary_Expression_Ast_Node);

  auto left = static_cast<Binary_Expression_Ast_Node*>(conditions->left.get());
  assert(left->op == "=");
  assert(left->left.get()->type == Ast_Node_Type::Ident_Expression_Ast_Node);
  assert(left->right.get()->type == Ast_Node_Type::Number_Literal_Expression_Ast_Node);

  auto right = static_cast<Binary_Expression_Ast_Node*>(conditions->right.get());
  assert(right->op == "like");
  assert(right->left.get()->type == Ast_Node_Type::Ident_Expression_Ast_Node);
  assert(right->right.get()->type == Ast_Node_Type::String_Literal_Expression_Ast_Node);
}

void test_ordered_map_01()
{
  Ordered_Map<std::string, size_t> map;

  assert(map.size() == 0);

  map.put("nome", 2);
  assert(map.size() == 1);

  map.put("nome", 3);
  assert(map.size() == 1);

  map.put("banana", 5);
  assert(map.size() == 2);

  assert(map.ordered_list.at(0).first == "nome");
  assert(map.ordered_list.at(0).second == 3);

  assert(map.ordered_list.at(1).first == "banana");
  assert(map.ordered_list.at(1).second == 5);

  map.clear();
  assert(map.size() == 0);
}

void test_value_aggregator_01()
{
  CSVData dummy_csv = make_dummy_csv();
  auto field_resolver = std::make_unique<Field_By_Name_Resolver>(dummy_csv, "number");

  std::unique_ptr<Aggregator> aggregator = std::make_unique<Value_Aggregator>(field_resolver);

  for (auto &data_row : dummy_csv.dataset)
  {
    aggregator->aggregate(&data_row);
  }

  assert(aggregator->type == Aggregator_Type::Values);
  auto value_aggregator = static_cast<Value_Aggregator*>(aggregator.get());

  assert(value_aggregator->size() == 3);

  assert(value_aggregator->ordered_data.ordered_list.at(0).first == "05");
  assert(value_aggregator->ordered_data.ordered_list.at(0).second.size() == 1);
  assert(value_aggregator->ordered_data.ordered_list.at(1).first == "03");
  assert(value_aggregator->ordered_data.ordered_list.at(1).second.size() == 2);
  assert(value_aggregator->ordered_data.ordered_list.at(2).first == "01");
  assert(value_aggregator->ordered_data.ordered_list.at(2).second.size() == 1);

}

void test_subgrouping_aggregator_02()
{
  CSVData dummy_csv = make_dummy_csv();
  auto id_field_resolver = std::make_unique<Field_By_Name_Resolver>(dummy_csv, "id");
  auto number_field_resolver = std::make_unique<Field_By_Name_Resolver>(dummy_csv, "number");

  std::unique_ptr<Aggregator> number_field_aggregator = std::make_unique<Value_Aggregator>(number_field_resolver);
  std::unique_ptr<Aggregator> root_aggregator = std::make_unique<Subgrouping_Aggregator>(id_field_resolver, number_field_aggregator);

  for (auto &data_row : dummy_csv.dataset)
  {
    root_aggregator->aggregate(&data_row);
  }

  assert(root_aggregator->type == Aggregator_Type::Subgrouping);
  auto subgrouping_aggregator = static_cast<Subgrouping_Aggregator*>(root_aggregator.get());

  assert(subgrouping_aggregator->size() == 4);

  assert(subgrouping_aggregator->ordered_data.ordered_list.at(0).first == "1");
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(0).second->size() == 1);
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(1).first == "2");
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(1).second->size() == 1);
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(2).first == "3");
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(2).second->size() == 1);
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(3).first == "4");
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(3).second->size() == 1);
}

void test_subgrouping_aggregator_03()
{
  CSVData dummy_csv = make_dummy_csv();
  auto id_field_resolver = std::make_unique<Field_By_Name_Resolver>(dummy_csv, "id");
  auto number_field_resolver = std::make_unique<Field_By_Name_Resolver>(dummy_csv, "number");

  std::unique_ptr<Aggregator> id_field_aggregator = std::make_unique<Value_Aggregator>(id_field_resolver);
  std::unique_ptr<Aggregator> root_aggregator = std::make_unique<Subgrouping_Aggregator>(number_field_resolver, id_field_aggregator);

  for (auto &data_row : dummy_csv.dataset)
  {
    root_aggregator->aggregate(&data_row);
  }

  assert(root_aggregator->type == Aggregator_Type::Subgrouping);
  auto subgrouping_aggregator = static_cast<Subgrouping_Aggregator*>(root_aggregator.get());

  assert(subgrouping_aggregator->size() == 3);

  assert(subgrouping_aggregator->ordered_data.ordered_list.at(0).first == "05");
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(0).second->size() == 1);
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(1).first == "03");
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(1).second->size() == 2);
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(2).first == "01");
  assert(subgrouping_aggregator->ordered_data.ordered_list.at(2).second->size() == 1);
}

int main()
{
  std::cout << "Iniciando testes" << std::endl << std::endl;

  test_ast_node_type();
  std::cout << "test_ast_node_type......................................OK" << std::endl;
  test_run_like_pattern_on();
  std::cout << "test_run_like_pattern_on................................OK" << std::endl;
  test_parse_describe_01();
  std::cout << "test_parse_describe_01..................................OK" << std::endl;
  test_parse_describe_02();
  std::cout << "test_parse_describe_02..................................OK" << std::endl;
  test_parse_select_01();
  std::cout << "test_parse_select_01....................................OK" << std::endl;
  test_ordered_map_01();
  std::cout << "test_ordered_map_01.....................................OK" << std::endl;
  test_value_aggregator_01();
  std::cout << "test_value_aggregator_01................................OK" << std::endl;
  test_subgrouping_aggregator_02();
  std::cout << "test_subgrouping_aggregator_02..........................OK" << std::endl;
  test_subgrouping_aggregator_03();
  std::cout << "test_subgrouping_aggregator_03..........................OK" << std::endl;

  std::cout << std::endl << "Fim testes" << std::endl;

  return EXIT_SUCCESS;
}
