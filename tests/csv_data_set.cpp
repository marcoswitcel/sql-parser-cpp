
#include "../lib/csv/src/csv.hpp"

CSVData make_dummy_csv()
{
  CSVData csv;

  csv.header.push_back("id");
  csv.header.push_back("number");
  csv.header.push_back("texto");

  auto row1 = CSV_Data_Row();
  row1.push_back("1");
  row1.push_back("05");
  row1.push_back("texto simples");
  csv.dataset.push_back(row1);

  auto row2 = CSV_Data_Row();
  row2.push_back("2");
  row2.push_back("03");
  row2.push_back("texto simples");
  csv.dataset.push_back(row2);

  auto row3 = CSV_Data_Row();
  row3.push_back("3");
  row3.push_back("03");
  row3.push_back("texto simples");
  csv.dataset.push_back(row3);

  auto row4 = CSV_Data_Row();
  row4.push_back("4");
  row4.push_back("01");
  row4.push_back("texto simples");
  csv.dataset.push_back(row4);

  return csv;
}