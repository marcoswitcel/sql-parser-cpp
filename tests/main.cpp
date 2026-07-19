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
#include "../src/collector_ast_node_visitor.hpp"
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
  assert(run_like_pattern_on("word", "___d"));
  assert(!run_like_pattern_on("word", "_____"));
  assert(!run_like_pattern_on("words", "____"));

  assert(run_like_pattern_on("words", "w%"));
  assert(run_like_pattern_on("words", "%rds"));
  assert(run_like_pattern_on("words", "%s"));
  assert(run_like_pattern_on("words", "word%"));
  assert(run_like_pattern_on("words", "%words%"));
  assert(run_like_pattern_on("words", "%w%o%r%d%s%"));
  assert(run_like_pattern_on("words", "%r%"));
  assert(run_like_pattern_on("words", "%_r_%"));
  assert(run_like_pattern_on("words", "%w%s%"));
  assert(run_like_pattern_on("words", "%w%ds%"));
  assert(!run_like_pattern_on("word", "%w%d_%"));

  assert(run_like_pattern_on("words", "w%%ords"));
  assert(run_like_pattern_on("words", "%%wo%%rds%%"));
  assert(run_like_pattern_on("words", "%%w%%rds%%"));

  assert(run_like_pattern_on("A small text sample", "%"));
  assert(run_like_pattern_on("A small text sample", "A small%"));
  assert(run_like_pattern_on("A small text sample", "%small%"));
  assert(run_like_pattern_on("A small text sample", "%sm_ll%"));
  assert(run_like_pattern_on("Moon", "%____%"));
  assert(!run_like_pattern_on("Car", "%____%"));

  // case insensitive tests
  assert(run_like_pattern_on("Word The Other Word", "%The%"));
  assert(!run_like_pattern_on("Word the Other Word", "%The%"));
  assert(run_like_pattern_on("Word The Other Word", "%the%"));
  assert(run_like_pattern_on("Word the Other Word", "%the%"));
}

void test_replace_char_with()
{
  const std::string sample = " oi com ' ";

  assert(replace_char_with(sample, '\'', "''") == " oi com '' ");
  assert(replace_char_with("ola", '\'', "''") == "ola");
  assert(replace_char_with("", '\'', "''") == "");
}

void test_ast_node_type()
{
  assert(ast_sub_type_of(Ast_Node_Type::Binary_Expression_Ast_Node, Ast_Node_Type::Expression_Ast_Node));
  assert(ast_sub_type_of(Ast_Node_Type::Ident_Expression_Ast_Node, Ast_Node_Type::Expression_Ast_Node));
  assert(ast_sub_type_of(Ast_Node_Type::String_Literal_Expression_Ast_Node, Ast_Node_Type::Expression_Ast_Node));
  assert(ast_sub_type_of(Ast_Node_Type::Number_Literal_Expression_Ast_Node, Ast_Node_Type::Expression_Ast_Node));
  assert(ast_sub_type_of(Ast_Node_Type::Function_Call_Expression_Ast_Node, Ast_Node_Type::Expression_Ast_Node));
}

void test_tokenizer()
{
  std::string sql = "Select teste as 'Teste renomeado', UPPER(nome) From tabela_teste ";
  SQL_Parse_Context parser(sql);

  Token token;
  
  token = parser.eat_token();
  assert(token.type == Token_Type::Select);
  assert(!parser.error);

  token = parser.eat_token();
  assert(token.type == Token_Type::Ident);
  assert(!parser.error);

  token = parser.eat_token();
  assert(token.type == Token_Type::As);
  assert(!parser.error);

  token = parser.eat_token();
  assert(token.type == Token_Type::String);
  assert(!parser.error);

  token = parser.eat_token();
  assert(token.type == Token_Type::Comma);
  assert(!parser.error);

  token = parser.eat_token();
  assert(token.type == Token_Type::Ident);
  assert(!parser.error);

  token = parser.eat_token();
  assert(token.type == Token_Type::Open_Parenthesis);
  assert(!parser.error);

  token = parser.eat_token();
  assert(token.type == Token_Type::Ident);
  assert(!parser.error);

  token = parser.eat_token();
  assert(token.type == Token_Type::Close_Parenthesis);
  assert(!parser.error);

  token = parser.eat_token();
  assert(token.type == Token_Type::From);
  assert(!parser.error);

  token = parser.eat_token();
  assert(token.type == Token_Type::Ident);
  assert(!parser.error);

  assert(!parser.is_finished());
  assert(!parser.error);

  parser.skip_whitespace();
  assert(parser.is_finished());
  assert(!parser.error);
}

void test_string_tokenizer()
{
  std::string sql = " 'teste string' ";
  SQL_Parse_Context parser(sql);

  Token token;
  
  token = parser.eat_token();
  assert(token.type == Token_Type::String);
  assert(!parser.error);

  // @todo João, continuar testando casos previsos no `try_parse_string`
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
  auto field_resolver = std::make_unique<Field_By_Name_Resolver>(dummy_csv.header, "number");

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

  auto group01 = value_aggregator->get_next_group_value();
  assert(group01.get() != nullptr);
  assert(group01->first.size() == 1);
  assert(group01->first.at(0) == "05");
  assert(group01->second.size() == 1);

  auto group02 = value_aggregator->get_next_group_value();
  assert(group02.get() != nullptr);
  assert(group02->first.size() == 1);
  assert(group02->first.at(0) == "03");
  assert(group02->second.size() == 2);

  auto group03 = value_aggregator->get_next_group_value();
  assert(group03.get() != nullptr);
  assert(group03->first.size() == 1);
  assert(group03->first.at(0) == "01");
  assert(group03->second.size() == 1);

  auto group04 = value_aggregator->get_next_group_value();
  assert(group04.get() == nullptr);
}

void test_subgrouping_aggregator_02()
{
  CSVData dummy_csv = make_dummy_csv();
  auto id_field_resolver = std::make_unique<Field_By_Name_Resolver>(dummy_csv.header, "id");
  auto number_field_resolver = std::make_unique<Field_By_Name_Resolver>(dummy_csv.header, "number");

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

  auto group01 = subgrouping_aggregator->get_next_group_value();
  assert(group01.get() != nullptr);
  assert(group01->first.size() == 2);
  assert(group01->second.size() == 1);

  auto group02 = subgrouping_aggregator->get_next_group_value();
  assert(group02.get() != nullptr);
  assert(group02->first.size() == 2);
  assert(group02->second.size() == 1);

  auto group03 = subgrouping_aggregator->get_next_group_value();
  assert(group03.get() != nullptr);
  assert(group03->first.size() == 2);
  assert(group03->second.size() == 1);

  auto group04 = subgrouping_aggregator->get_next_group_value();
  assert(group04.get() != nullptr);
  assert(group04->first.size() == 2);
  assert(group04->second.size() == 1);

  auto group05 = subgrouping_aggregator->get_next_group_value();
  assert(group05.get() == nullptr);
}

void test_subgrouping_aggregator_03()
{
  CSVData dummy_csv = make_dummy_csv();
  auto id_field_resolver = std::make_unique<Field_By_Name_Resolver>(dummy_csv.header, "id");
  auto number_field_resolver = std::make_unique<Field_By_Name_Resolver>(dummy_csv.header, "number");

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

  assert(root_aggregator->grouping_depth() == 2);

  auto group01 = subgrouping_aggregator->get_next_group_value();
  assert(group01.get() != nullptr);
  assert(group01->first.size() == 2);
  assert(group01->first.at(0) == "05");
  assert(group01->first.at(1) == "1");
  assert(group01->second.size() == 1);

  auto group02 = subgrouping_aggregator->get_next_group_value();
  assert(group02.get() != nullptr);
  assert(group02->first.size() == 2);
  assert(group02->first.at(0) == "03");
  assert(group02->first.at(1) == "2");
  assert(group02->second.size() == 1);

  auto group03 = subgrouping_aggregator->get_next_group_value();
  assert(group03.get() != nullptr);
  assert(group03->first.size() == 2);
  assert(group03->first.at(0) == "03");
  assert(group03->first.at(1) == "3");
  assert(group03->second.size() == 1);

  auto group04 = subgrouping_aggregator->get_next_group_value();
  assert(group04.get() != nullptr);
  assert(group04->first.size() == 2);
  assert(group04->first.at(0) == "01");
  assert(group04->first.at(1) == "4");
  assert(group04->second.size() == 1);

  auto group05 = subgrouping_aggregator->get_next_group_value();
  assert(group05.get() == nullptr);
}

void test_run_sql_on_csv01()
{
  CSVData dummy_csv = make_dummy_csv();
  
  assert(dummy_csv.header.size() == 3);
  assert(dummy_csv.dataset.size() == 4);

  SQL_Parse_Context parser("SELECT id, number, texto, SUBSTRING(texto, 0, 5) as \"texto cortado\" FROM dummy ");

  Ast_Node* node = parser.eat_node();
  assert(node && node->type == Ast_Node_Type::Select_Ast_Node);

  auto select = dynamic_cast<Select_Ast_Node*>(node);
  /**
   * A função `run_select_on_csv` tem diversos asserts e logs para o console, se
   * ela não emitiu nenhum log e não disparou o assert, num geral entendesse que
   * a operação executou corretamente.
   * 
   * @note evoluir esses testes no futuro
   * 
   */
  assert(run_select_on_csv(*select, dummy_csv, false));


  assert(dummy_csv.header.size() == 4);
  assert(dummy_csv.header.at(0) == "id");
  assert(dummy_csv.header.at(1) == "number");
  assert(dummy_csv.header.at(2) == "texto");
  assert(dummy_csv.header.at(3) == "texto cortado");
  assert(dummy_csv.dataset.size() == 4);
  for (auto &row: dummy_csv.dataset)
  {
    assert(row.size() == 4);
    assert(row.at(3).size() < 6);
  }
}

void test_run_sql_on_csv02()
{
  CSVData dummy_csv = make_dummy_csv();

  SQL_Parse_Context parser("SELECT number, COUNT(*) FROM dummy Group By number");

  Ast_Node* node = parser.eat_node();
  assert(node && node->type == Ast_Node_Type::Select_Ast_Node);

  auto select = dynamic_cast<Select_Ast_Node*>(node);
  /**
   * A função `run_select_on_csv` tem diversos asserts e logs para o console, se
   * ela não emitiu nenhum log e não disparou o assert, num geral entendesse que
   * a operação executou corretamente.
   * 
   * @note evoluir esses testes no futuro
   * 
   */
  assert(run_select_on_csv(*select, dummy_csv, false));

  assert(dummy_csv.header.size() == 2);
  assert(dummy_csv.dataset.size() == 3);
}

void test_run_sql_on_csv03()
{
  CSVData dummy_csv = make_dummy_csv();

  SQL_Parse_Context parser("SELECT number, COUNT(*) FROM dummy Where id = 1 Group By number");

  Ast_Node* node = parser.eat_node();
  assert(node && node->type == Ast_Node_Type::Select_Ast_Node);

  auto select = dynamic_cast<Select_Ast_Node*>(node);
  /**
   * A função `run_select_on_csv` tem diversos asserts e logs para o console, se
   * ela não emitiu nenhum log e não disparou o assert, num geral entendesse que
   * a operação executou corretamente.
   * 
   * @note evoluir esses testes no futuro
   * 
   */
  assert(run_select_on_csv(*select, dummy_csv, false));

  assert(dummy_csv.header.size() == 2);
  assert(dummy_csv.dataset.size() == 1);
  assert(dummy_csv.dataset.at(0).at(0) == "05");
  assert(dummy_csv.dataset.at(0).at(1) == "1");
}

void test_collector_ast_node_visitor()
{
  SQL_Parse_Context parser("SELECT Id, Name, 'valor fixo' As \"Ident With Spaces\"  FROM Dummy Where Id > 50");
  
  Ast_Node* node = parser.eat_node();
  
  assert(node && node->type == Ast_Node_Type::Select_Ast_Node);
  auto select = static_cast<Select_Ast_Node*>(node);

  Collector_Ast_Node_Visitor collector;

  select->accept(collector);

  assert(collector.froms.size() == 1);
  assert(collector.idents.size() == 3);
  assert(collector.numbers.size() == 1);
  assert(collector.strings.size() == 1);
}

/**
 * @brief todos os testes são cadastrados aqui
 * 
 * @return int 
 */
int main()
{
  std::cout << "Iniciando testes" << std::endl << std::endl;

  test_ast_node_type();
  std::cout << "test_ast_node_type......................................OK" << std::endl;
  test_run_like_pattern_on();
  std::cout << "test_run_like_pattern_on................................OK" << std::endl;
  test_tokenizer();
  std::cout << "test_tokenizer..........................................OK" << std::endl;
  test_replace_char_with();
  std::cout << "test_replace_char_with..................................OK" << std::endl;
  test_string_tokenizer();
  std::cout << "test_string_tokenizer...................................OK" << std::endl;
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
  test_run_sql_on_csv01();
  std::cout << "test_run_sql_on_csv01...................................OK" << std::endl;
  test_run_sql_on_csv02();
  std::cout << "test_run_sql_on_csv02...................................OK" << std::endl;
  test_run_sql_on_csv03();
  std::cout << "test_run_sql_on_csv03...................................OK" << std::endl;
  test_collector_ast_node_visitor();
  std::cout << "test_collector_ast_node_visitor.........................OK" << std::endl;
  

  std::cout << std::endl << "Fim testes" << std::endl;

  return EXIT_SUCCESS;
}
