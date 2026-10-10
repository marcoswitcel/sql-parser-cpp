#pragma once

#include <string>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <ctime>
#include <vector>
#include <chrono>
#include <algorithm>

#include "./utils.cpp"
#include "./ast_node.hpp"
#include "./aggregator.cpp"
#include "./collector_ast_node_visitor.hpp"
#include "./resolver.cpp"
#include "./ordered_map.hpp"
// Dependências
#include "../lib/csv/src/csv.hpp"


using std::vector;


bool run_like_pattern_on_internal(const std::string &text_input, size_t input_index_parameter, const std::string &like_pattern, size_t pattern_index_parameter)
{
  size_t input_index = input_index_parameter;
  size_t pattern_index = pattern_index_parameter;

  while (input_index < text_input.size())
  {
    if (pattern_index >= like_pattern.size()) return false;

    auto &pattern_char = like_pattern[pattern_index];
    auto &text_char = text_input[input_index];

    if (pattern_char == '%')
    {
      if (pattern_index + 1 == like_pattern.size())
      {
        return true;
      }
      
      if (run_like_pattern_on_internal(text_input, input_index, like_pattern, pattern_index + 1))
      {
        return true;
      }

      input_index++;
    }
    else if (pattern_char == '_')
    {
      input_index++;
      pattern_index++;
    }
    else
    {
      if (pattern_char != text_char) return false;

      input_index++;
      pattern_index++;
    } 
  }

  if (input_index < text_input.size()) return false;

  if (pattern_index >= like_pattern.size()) return true;

  for (; pattern_index < like_pattern.size(); pattern_index++)
  {
    auto &pattern_char = like_pattern[pattern_index];
    if (pattern_char != '%') return false;
  }

  return true;
}

bool run_like_pattern_on(std::string text_input, std::string raw_like_pattern)
{
  return run_like_pattern_on_internal(text_input, 0, raw_like_pattern, 0);
}

bool extract_lhs_and_rhs_expressions(
  Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row,
  std::string &lhs, std::string &rhs)
{
  Expression_Resolver resolver_left = Expression_Resolver(&csv.header, node->left.get());
  Expression_Resolver resolver_right = Expression_Resolver(&csv.header, node->right.get());

  lhs = resolver_left.resolve(data_row);
  rhs = resolver_right.resolve(data_row);

  return true;
}

bool evaluate_equals_binary_ast_node(Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row)
{
  assert(node->op == Binary_Operation::Equals || node->op == Binary_Operation::Diferent);

  std::string lhs = "";
  std::string rhs = "";
  if (!extract_lhs_and_rhs_expressions(node, csv, data_row, lhs, rhs))
  {
    return false;
  }

  // Quando chega nessa etapa o método infer_type das instâncias de `Function_Call_Expression_Ast_Node` já deve ter sido chamado
  // @note seria legal retornar o valor bruto ao invés das strings... muita conversão desnecessária
  if (node->left->inferred_type == Inferred_Type::Number &&
    node->right->inferred_type == Inferred_Type::Number)
  {
    return std::stof(lhs) == std::stof(rhs);
  }

  // @todo João, não lida com números, possivelmente se aplica em outras sessões também
  return lhs.compare(rhs) == 0;
}

float evaluate_compare_binary_ast_node(Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row)
{
  assert(node->op == Binary_Operation::Greater_Than || node->op == Binary_Operation::Lower_Than);

  std::string lhs = "";
  std::string rhs = "";
  if (!extract_lhs_and_rhs_expressions(node, csv, data_row, lhs, rhs))
  {
    return 0;
  }

  // @todo João, completamente errado... precisa considerar strings inválidas e principalmente, precisa retornar os números diretamente
  return std::stof(lhs) - std::stof(rhs);
}

bool evaluate_not_equals_binary_ast_node(Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row)
{
  return !evaluate_equals_binary_ast_node(node, csv, data_row);
}

bool evaluate_like_binary_ast_node(Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row)
{
  assert(node->op == Binary_Operation::Like || node->op == Binary_Operation::Not_Like);

  std::string lhs = "";
  std::string rhs = "";
  if (!extract_lhs_and_rhs_expressions(node, csv, data_row, lhs, rhs))
  {
    return false;
  }
  
  return run_like_pattern_on(lhs, rhs);
}

bool evaluate_relational_binary_ast_node(Binary_Expression_Ast_Node* node, CSVData &csv, std::vector<std::string> &data_row)
{
  if (node->op == Binary_Operation::Equals)
  {
    return evaluate_equals_binary_ast_node(node, csv, data_row);
  }
  else if (node->op == Binary_Operation::Lower_Than)
  {
    return evaluate_compare_binary_ast_node(node, csv, data_row) < 0;
  }
  else if (node->op == Binary_Operation::Greater_Than)
  {
    return evaluate_compare_binary_ast_node(node, csv, data_row) > 0;
  }
  else if (node->op == Binary_Operation::Diferent)
  {
    return evaluate_not_equals_binary_ast_node(node, csv, data_row);
  }
  else if (node->op == Binary_Operation::Like)
  {
    return evaluate_like_binary_ast_node(node, csv, data_row);
  }
  else if (node->op == Binary_Operation::Not_Like)
  {
    return !evaluate_like_binary_ast_node(node, csv, data_row);
  }
  else if (node->op == Binary_Operation::Or)
  {
    return evaluate_relational_binary_ast_node(static_cast<Binary_Expression_Ast_Node *>(node->left.get()), csv, data_row) ||
      evaluate_relational_binary_ast_node(static_cast<Binary_Expression_Ast_Node *>(node->right.get()), csv, data_row);
  }
  else if (node->op == Binary_Operation::And)
  {
    return evaluate_relational_binary_ast_node(static_cast<Binary_Expression_Ast_Node *>(node->left.get()), csv, data_row) &&
      evaluate_relational_binary_ast_node(static_cast<Binary_Expression_Ast_Node *>(node->right.get()), csv, data_row);
  }

  // @note o sistema não deve parsear nada diferente das opções acima, porém se o "if-else" ficar
  // desatualizado pode cair no fluxo abaixo, no build de dev alerta com o assert 
  assert(false);
  return false;
}



bool does_field_exist(CSVData &csv, std::string field_name)
{
  auto it = std::find(csv.header.begin(), csv.header.end(), field_name);
  
  return it != csv.header.end();
}

bool is_where_valid_for_execution(Binary_Expression_Ast_Node* node)
{
  auto left_expr = node->left.get();
  auto right_expr = node->right.get();
  bool is_left_valid = true;
  bool is_right_valid = true;

  
  if (auto bin_expr = Cast_If(Binary_Expression_Ast_Node, *left_expr)) {
    is_left_valid = is_where_valid_for_execution(bin_expr);
  }
  else if (auto call_expr = Cast_If(Function_Call_Expression_Ast_Node, *left_expr))
  {
    is_left_valid = known_function_name_and_argument_list(call_expr);

    // se válido no primeiro nível checa recursivamente
    if (is_left_valid)
    {
      // @note por hora o método `is_arguments_valid` não precisa dos cabeçalhos...
      Tabular_Data_Header header;
      auto function_call_resolver = new Function_Call_Expression_Resolver(&header, call_expr);

      is_left_valid = function_call_resolver->is_arguments_valid();
    }
  }


  if (auto bin_expr = Cast_If(Binary_Expression_Ast_Node, *right_expr)) {
    is_right_valid = is_where_valid_for_execution(bin_expr);
  }
  else if (auto call_expr = Cast_If(Function_Call_Expression_Ast_Node, *right_expr))
  {
    is_right_valid = known_function_name_and_argument_list(call_expr);

    // se válido no primeiro nível checa recursivamente
    if (is_right_valid)
    {
      // @note por hora o método `is_arguments_valid` não precisa dos cabeçalhos...
      Tabular_Data_Header header;
      auto function_call_resolver = new Function_Call_Expression_Resolver(&header, call_expr);

      is_right_valid = function_call_resolver->is_arguments_valid();
    }
  }


  if (!is_left_valid)
  {
    std::cout << "A expressão a seguir não pode ser interpretada: " << std::endl << left_expr->to_expression() << std::endl;
  }

  if (!is_right_valid)
  {
    std::cout << "A expressão a seguir não pode ser interpretada: " << std::endl << right_expr->to_expression() << std::endl;
  }


  return is_left_valid && is_right_valid;
}

bool is_all_idents_valid_in_select(Collector_Ast_Node_Visitor &collector, CSVData &csv, bool is_verbose)
{
  for (auto entry : collector.idents)
  {
    if (entry.type != Section_Type_Entry::Select) continue;

    std::string &field = entry.ident;
    
    if (field  == "*") continue;

    if (!does_field_exist(csv, field))
    {
      if (is_verbose)
      {
        std::cout << "Error: campo '" << field << "' requisitado no Select não existe no csv." << std::endl;
      }
      return false;
    }
  }

  return true;
}

bool is_all_idents_valid_in_where(Collector_Ast_Node_Visitor &collector, CSVData &csv, bool is_verbose)
{
  for (auto entry : collector.idents)
  {
    if (entry.type != Section_Type_Entry::Where) continue;

    std::string &field = entry.ident;

    if (!does_field_exist(csv, field))
    {
      bool found = false;
      for (auto entry : collector.idents)
      {
        // se tiver um 'alias', pode prosseguir
        if (entry.type == Section_Type_Entry::Select && entry.as == field)
        {
          found = true;
          continue;
        }
      }

      if (found) continue;

      if (is_verbose)
      {
        std::cout << "Error: campo '" << field << "' requisitado no Where não existe no csv." << std::endl;
      }
      return false;
    }
  }

  return true;
}

/**
 * @brief 
 * 
 * @param collector 
 * @param csv essa instância já terá passado pela manipulação para computar os valores da tabela, então encontrará os alias válidos 
 * @return true 
 * @return false 
 */
bool is_all_idents_valid_order_by(Collector_Ast_Node_Visitor &collector, CSVData &csv, bool is_verbose)
{
  for (auto entry : collector.idents)
  {
    if (entry.type != Section_Type_Entry::Order_By) continue;

    std::string &field = entry.ident;

    if (!does_field_exist(csv, field))
    {
      if (is_verbose)
      {
        std::cout << "Error: campo '" << field << "' requisitado no Order By não existe no csv." << std::endl;
      }
      return false;
    }
  }

  return true;
}

bool run_select_on_csv(Select_Ast_Node &select, CSVData &csv, bool is_printing_as_table)
{

  Collector_Ast_Node_Visitor collector;

  select.accept(collector);

  if (!is_all_idents_valid_in_select(collector, csv, is_printing_as_table)) return false;

  vector<std::string> new_header;
  vector<Field_Resolver*> field_resolver;
  
  for (auto field : select.fields)
  {
    field->infer_type();
    
    if (field->type == Ast_Node_Type::Ident_Expression_Ast_Node)
    {
      auto ident = static_cast<Ident_Expression_Ast_Node*>(field.get());
  
      if (ident->ident_name == "*")
      {
        for (auto column : csv.header)
        {
          new_header.push_back(column);
          field_resolver.push_back(new Field_By_Name_Resolver(csv.header, column));
        }
      }
      else
      {
        if (!contains(csv.header, ident->ident_name))
        {
          std::cout << "Coluna inexistente no dataset: " << ident->ident_name << std::endl;
          return false;
        }
  
        if (ident->as.empty())
        {
          new_header.push_back(ident->ident_name);
        }
        else
        {
          new_header.push_back(ident->as);
        }
        field_resolver.push_back(new Field_By_Name_Resolver(csv.header, ident->ident_name));
      }
    }
    else if (field->type == Ast_Node_Type::String_Literal_Expression_Ast_Node)
    {
      auto string = static_cast<String_Literal_Expression_Ast_Node*>(field.get());
      if (string->as.empty())
      {
        new_header.push_back(string->value);
      }
      else
      {
        new_header.push_back(string->as);
      }
      field_resolver.push_back(new String_Literal_Resolver(string->value));
    }
    else if (field->type == Ast_Node_Type::Number_Literal_Expression_Ast_Node)
    {
      auto number = static_cast<Number_Literal_Expression_Ast_Node*>(field.get());
      if (number->as.empty())
      {
        new_header.push_back(std::to_string(number->value));
      }
      else
      {
        new_header.push_back(number->as);
      }
      field_resolver.push_back(new Number_Literal_Resolver(number->value));
    }
    else if (field->type == Ast_Node_Type::Binary_Expression_Ast_Node && static_cast<Binary_Expression_Ast_Node*>(field.get())->op == Binary_Operation::Concat)
    {
      auto bin_expr = static_cast<Binary_Expression_Ast_Node*>(field.get());
      if (bin_expr->as.empty())
      {
        new_header.push_back(bin_expr->to_expression());
      }
      else
      {
        new_header.push_back(bin_expr->as);
      }
      field_resolver.push_back(new Binary_Expression_Resolver(&csv.header, bin_expr));
    }
    else if (field->type == Ast_Node_Type::Function_Call_Expression_Ast_Node && known_function_name_and_argument_list(static_cast<Function_Call_Expression_Ast_Node*>(field.get())))
    {
      auto call_expr = static_cast<Function_Call_Expression_Ast_Node*>(field.get());
      if (call_expr->as.empty())
      {
        new_header.push_back(call_expr->to_expression());
      }
      else
      {
        new_header.push_back(call_expr->as);
      }

      auto function_call_resolver = new Function_Call_Expression_Resolver(&csv.header, call_expr);

      if (function_call_resolver->is_arguments_valid())
      {
        field_resolver.push_back(function_call_resolver);
      }
      else
      {
        delete function_call_resolver;
        std::cout << "A expressão a seguir não pode ser interpretada: " << std::endl << field->to_expression() << std::endl;
        return false;
      }
    }
    else
    {
      std::cout << "A expressão a seguir não pode ser interpretada: " << std::endl << field->to_expression() << std::endl;
      return false;
    }
  }

  assert(new_header.size() == field_resolver.size());

  if (csv.dataset.size() == 0)
  {
    for (Field_Resolver* it : field_resolver) delete it;
    
    return false;
  }

  if (select.where && !is_where_valid_for_execution(select.where->conditions.get()))
  {
    return false;
  }

  bool has_aggregation_function = false;
  
  for (auto &field : select.fields)
  {
    if (auto func = Cast_If(Function_Call_Expression_Ast_Node, *field))
    {
      if (is_an_aggregation_funcion(func->tagged_name))
      {
        has_aggregation_function = true;
      }
    }
  }

  if (!is_all_idents_valid_in_where(collector, csv, is_printing_as_table)) return false;
  
  const auto has_where = select.where && select.where->conditions.get();
  const auto has_group_by = (select.group_by && select.group_by->groups.size() > 0);
  const auto has_order_by = (select.order_by && select.order_by->orders.size() > 0);
  vector<CSV_Data_Row> new_dataset;
  std::unique_ptr<Aggregator> root_aggregator;
  
  if (has_group_by || has_aggregation_function)
  {
    // @note em caso de group by pode identificadores mas no caso de apenas o COUNT ou funções de agregação não pode
    // Mais pra baixo é checada essa diferença e bloqueado os campos inválidos.
    for (auto &field : select.fields)
    {
      if (auto ident = Cast_If(Ident_Expression_Ast_Node, *field))
      {
        bool found = false;

        for (auto &grouping_field : select.group_by->groups)
        {
          if (auto group_by_ident = Cast_If(Ident_Expression_Ast_Node, *grouping_field))
          {
            if (group_by_ident->ident_name == ident->ident_name)
            {
              found = true;
              continue;
            } 
          }
          else
          {
            std::cout << "Por hora todas as expressões no Group By precisam ser identificadores simples." << std::endl;
            return false;
          }
        }
        
        if (!found)
        {
          if (is_printing_as_table)
          {
            std::cout << "Por hora todos os identificadores do select precisam estar contidos na cláusula Group By." << std::endl;
          }
          return false;
        }
      }
      else if (auto func = Cast_If(Function_Call_Expression_Ast_Node, *field))
      {
        if (!is_an_aggregation_funcion(func->tagged_name))
        {
          std::cout << "Por hora todas as chamadas de funções precisam ser para funções de agregação. Apenas COUNT, MAX, MIN e etc... são suportados." << std::endl;
          return false;
        }
      }
      else
      {
        std::cout << "Por hora todos os campos do select precisam ser compostos apenas por identificadores ou funções agregadoras." << std::endl;
        return false;
      }
    }
    
    // montando estrutura de agregadores, no caso de ter group by
    if (has_group_by)
    {
      for (size_t i = select.group_by->groups.size(); i > 0; i--)
      {
        std::unique_ptr<Expression_Ast_Node> &grouping_field = select.group_by->groups.at(i - 1);
  
        if (auto ident = Cast_If(Ident_Expression_Ast_Node, *grouping_field))
        {
          auto &field_name = ident->ident_name;
          auto field_resolver = std::make_unique<Field_By_Name_Resolver>(csv.header, field_name);
          
          if (root_aggregator)
          {
            auto aggregator = std::make_unique<Subgrouping_Aggregator>(field_resolver, root_aggregator);
            root_aggregator = std::move(aggregator);
          }
          else
          {
            auto aggregator = std::make_unique<Value_Aggregator>(field_resolver);
            root_aggregator = std::move(aggregator);
          }
        }
        else
        {
          std::cout << "A expressão a seguir não pode ser aplicada no Group By: " << std::endl << grouping_field->to_expression() << std::endl;
          return false;
        }
      }
    }
    
    if (has_group_by)
    {
      // executando processo de agregação
      for (CSV_Data_Row &data_row: csv.dataset)
      {
        if (has_where)
        {
          if (!evaluate_relational_binary_ast_node(select.where->conditions.get(), csv, data_row))
          {
            continue;
          }
        }
        
        root_aggregator->aggregate(&data_row);
      }
      
      auto grouping_header = root_aggregator->get_header();
      vector<std::unique_ptr<Aggregation_Field_Resolver>> field_aggregation_resolvers;
      
      for (auto &field : select.fields)
      {
        if (auto ident = Cast_If(Ident_Expression_Ast_Node, *field))
        {
          field_aggregation_resolvers.push_back(std::make_unique<Field_By_Name_Aggregation_Resolver>(*grouping_header, ident->ident_name));
        }
        else if (auto func = Cast_If(Function_Call_Expression_Ast_Node, *field))
        {
          field_aggregation_resolvers.push_back(std::make_unique<Function_Call_Expression_Aggregation_Resolver>(grouping_header.get(), &csv.header, func));
        }
      }
      
      while (auto value = root_aggregator->get_next_group_value())
      {
        std::vector<std::string> new_data_row;
  
        for (auto &resolver : field_aggregation_resolvers)
        {
          new_data_row.push_back(resolver->resolve(value->first, value->second));
        }
    
        new_dataset.push_back(new_data_row);
      }
    }
    else // se apenas tem funções de agregação
    {
      
      for (auto &field : select.fields)
      {
        if (auto func = Cast_If(Function_Call_Expression_Ast_Node, *field))
        {
          if (!is_an_aggregation_funcion(func->tagged_name))
          {
            std::cout << "Por hora apenas funções de agregação são suportadas." << std::endl;
            return false;  
          }
        }
        else
        {
          std::cout << "Por hora todos os campos do select precisam ser compostos apenas por funções agregadoras quando houver função agregadora." << std::endl;
          return false;
        }
      }

      vector<CSV_Data_Row*> subset_dataset;

      // executando processo de agregação
      for (CSV_Data_Row &data_row: csv.dataset)
      {
        if (has_where)
        {
          if (!evaluate_relational_binary_ast_node(select.where->conditions.get(), csv, data_row))
          {
            continue;
          }
        }
        
        subset_dataset.push_back(&data_row);
      }

      std::unique_ptr<Tabular_Data_Header> grouping_header;
      vector<std::unique_ptr<Aggregation_Field_Resolver>> field_aggregation_resolvers;
      
      for (auto &field : select.fields)
      {
        if (auto func = Cast_If(Function_Call_Expression_Ast_Node, *field))
        {
          field_aggregation_resolvers.push_back(std::make_unique<Function_Call_Expression_Aggregation_Resolver>(grouping_header.get(), &csv.header, func));
        }
        else
        {
          // @note por hora entendo que se chegou aqui só terá funções de agregação, porém, não lembro se na especificação
          // SQL existe alguma forma de mencionar campos sem ter o Group By explícito no select, quando for apenas com funções
          // acredito que todos os campos serão expressões baseadas em funções de agregação mesmo.
          assert(false);
        }
      }

      // montando linha única
      std::vector<std::string> new_data_row;
  
      for (auto &resolver : field_aggregation_resolvers)
      {
        new_data_row.push_back(resolver->resolve(*grouping_header, subset_dataset));
      }
  
      new_dataset.push_back(new_data_row);
    }
  }
  else
  {
    // caminho rápido quando não há agregador
    for (CSV_Data_Row &data_row: csv.dataset)
    {
      if (has_where)
      {
        if (!evaluate_relational_binary_ast_node(select.where->conditions.get(), csv, data_row))
        {
          continue;
        }
      }
  
      std::vector<std::string> new_data_row;
      
      for (auto resolver : field_resolver)
      {
        new_data_row.push_back(resolver->resolve(data_row));
      }
  
      new_dataset.push_back(new_data_row);
    }
  }

  // @note João, essa ideia de alterar o csv de entrada é estranha, vou usar isso nos testes, porém
  // seria interessante pensar em algo melhor para o futuro. E.x: Copia ou receber um csv para gravar as linhas
  csv.header = new_header;
  csv.dataset = new_dataset;

  if (has_order_by)
  {
    // @todo João, tem um erro aqui ainda... pelo menos um... quando o csv chega aqui ele já foi manipulado
    // e o order by deve poder acessar campos fora da lista de campos que devem ser retornardos em tela, ele pode
    // usar uma coluna não visível para ordenação. Tem um diferença quando tem group by, nesse caso a lista de campos
    // o definidas no select são todos o que podem ser usados no order by.
    // @note Isso aqui funciona, porém, deixa processar muita coisa para dizer que um Ident declarado no Order_By
    // não existe nas colunas esperadas no retorno 
    if (!is_all_idents_valid_order_by(collector, csv, is_printing_as_table)) return false;

    auto &order_expr = select.order_by->orders.at(0);
  
    auto column_index = 0;
    
    if (order_expr->expr->type == Ast_Node_Type::Number_Literal_Expression_Ast_Node)
    {
      auto number = static_cast<Number_Literal_Expression_Ast_Node*>(order_expr->expr.get());
      column_index = number->value;
      // @note o valor `0` é refeitado no parse, se não for causará problemas, por isso os assert's que seguem
      assert(column_index > 0);

      // decremente porque recebemos 1 para primeira coluna mas no array é 0 a primeira
      column_index--;
    }
    else
    {
      assert(order_expr->expr->type == Ast_Node_Type::Ident_Expression_Ast_Node);
      auto ident = static_cast<Ident_Expression_Ast_Node*>(order_expr->expr.get());

      auto it = std::find(csv.header.begin(), csv.header.end(), ident->ident_name);
      
      if (it == csv.header.end())
      {
        std::cout << "Error: field_name: " << ident->ident_name << " não existe na tabela." << std::endl;
        assert(false);
      }

      // @todo João, falta validar se o campo existe na lista de campos declarados, ou pode ordenar por um campo não solicitado?
      column_index = std::distance(csv.header.begin(), it);
    }
    
    assert(column_index >= 0);

    assert(static_cast<size_t>(column_index) < csv.header.size());

    if (order_expr->dir == Token_Type::Asc)
    {
      std::sort(csv.dataset.begin(), csv.dataset.end(), [column_index](const CSV_Data_Row &a, const CSV_Data_Row &b) {
        return a.at(column_index) < b.at(column_index);
      });
    }
    else
    {
      std::sort(csv.dataset.begin(), csv.dataset.end(), [column_index](const CSV_Data_Row &a, const CSV_Data_Row &b) {
        return a.at(column_index) > b.at(column_index);
      });
    }
  }

  if (is_printing_as_table)
  {
    print_as_table(csv, Columns_Print_Mode::All_Columns, NULL, 30);
  }
  
  for (Field_Resolver* it : field_resolver) delete it;

  return true;
}

bool run_describe_on_csv(Describe_Ast_Node &describe, CSVData &csv)
{
  csv.infer_types();

  CSV_Data_Row header = { "Column Name", "Type", "Nullable" };
  std::vector<CSV_Data_Row> dataset;

  for (size_t i = 0; i < csv.header.size(); i++)
  {
    auto &col_info = csv.infered_types_for_columns.at(i);
    CSV_Data_Row new_row;
    
    new_row.push_back(csv.header.at(i));
    new_row.push_back(to_string(col_info.type));
    new_row.push_back(col_info.nullable ? "yes" : "No");

    dataset.push_back(new_row);
  }

  std::cout << "Describe of table: " << describe.ident_name->to_expression() << std::endl;
  print_as_table(header, dataset, Columns_Print_Mode::All_Columns, NULL, 30);
  
  return false;
}
