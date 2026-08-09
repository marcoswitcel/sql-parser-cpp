#pragma once

#include <string.h>
#include <utility>

/**
 * @brief permite encontrar o índice de um dado parâmetro caso ele exista, caso não,
 * retorna -1, o que permite identificar a presença ou não do argumento.
 * 
 * @param argument 
 * @param argc 
 * @param argv 
 * @return int 
 */
int index_of_in_argv(const char *argument, int argc, const char *argv[])
{
  for (int i = 0; i < argc; i++)
  {
    if (!strcmp(argv[i], argument))
    {
      return i;
    }
  }
  return -1;
}

bool is_string_present_in_argv(const char *switch_name, int argc, const char *argv[])
{
  for (int i = 0; i < argc; i++)
  {
    if (!strcmp(argv[i], switch_name))
    {
      return true;
    }
  }
  return false;
}

struct Found_Value {
  /**
   * @brief indica se o 'arg_name' foi encontrado
   * 
   */
  bool found;
  /**
   * @brief indica se tinha um tinha um cstring associada a esse 'arg_name', NULL para não encontrado
   * e o endereço para caso tenha encontrado
   * 
   */
  const char* value;
};

Found_Value get_value_for_in_argv(const char *arg_name, int argc, const char *argv[])
{
  for (int i = 0; i < argc; i++)
  {
    if (!strcmp(argv[i], arg_name))
    {
      int next_index = i + 1;
      const char* value = NULL;
      if (next_index < argc)
      {
        value = argv[next_index];
      }

      return { .found = true, .value = value };
    }
  }
  return { .found = false, .value = NULL };
}
