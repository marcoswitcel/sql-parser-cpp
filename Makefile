# Configurações de compilação
## usando g++ não precisa do parâmetro -lstdc++
CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++2a -g
LFLAGS = -Llib -lcsv
DEF = -DDEV_CODE_ENABLED

# Configurações gerais
SOURCE_FOLDER_NAME=src
TESTS_FOLDER_NAME=tests
BUILD_FOLDER_NAME=target

#RUN_ARGS= "SELECT * FROM DUAL" --verbose
#RUN_ARGS= "Select Name, Phone1 , Phone2 From customers Where Name = 'Marcelson' or Name = 'Jucelson' or 'Brunelson' = Name " --verbose #--print-tokens
#RUN_ARGS= "Select Name, Phone1 , Phone2 From customers Where Name = 'Marcelson' OR Name = 'Jucelson' " --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "Select Runtime, Series_Title, Released_Year, Runtime From customers Where Runtime = '146 min' " --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
# RUN_ARGS= "Select Runtime, Series_Title, Released_Year, Runtime From customers Where Released_Year = '2013' " --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "Select Runtime, Series_Title, Released_Year, Runtime From imdb_top_1000 Where Series_Title like 'The%' " --verbose  --bind "imdb_top_1000=imdb_top_1000.csv"  #--print-tokens
#RUN_ARGS= "Select * From customers Where Name like '%celson%' " --verbose  --csv-filename example.csv #--print-tokens
#RUN_ARGS= "Select Phone1 as abc, Phone2, Name, Name From customers Where Name like '%celson%' " --verbose  --csv-filename example.csv #--print-tokens
#RUN_ARGS= "Describe teste " --verbose  --csv-filename example.csv #--print-tokens
#RUN_ARGS= "Describe csv_file " --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT Series_Title, Gross from imdb_top_1000 where Gross = '' " --bind imdb_top_1000=imdb_top_1000.csv --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT Series_Title, Gross from imdb_top_1000 " --bind "imdb_top_1000=imdb_top_1000.csv,teste=teste.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "Select Name, Phone1 , Phone2 From customers Where Name = 02" --verbose --print-tokens
#RUN_ARGS= "Select * From customers " --verbose --bind "customers=customers-100.csv"
#RUN_ARGS= "Select Index, Company, City, Country, Email, Website From customers where Country = 'Canada' "  --verbose --bind "customers=customers-100.csv"
#RUN_ARGS= "Select Index, \"First Name\", \"Last Name\", \"Last Name\" as Last_Name_Computed, 'teste' as Teste, Company, City, Country, Email, Website From customers where Country = 'Canada' "  --verbose --bind "customers=customers-100.csv"
#RUN_ARGS= "Select Index, \"First Name\" || ' ' || \"Last Name\"  as \"Full Name\", Company, City, Country, Email, Website From customers where Country = 'Canada' "  --verbose --bind "customers=customers-100.csv"
#RUN_ARGS= "Select Index, \"First Name\" || ' ' || \"Last Name\", Company, City, Country, Email, Website From customers where Country = 'Canada' "  --verbose --bind "customers=customers-100.csv"
#RUN_ARGS= "Select Index, \"First Name\" || ' ' || \"Last Name\" as \"Full Name\" , 'today:' || CURRENT_DATE() as Today, Company, Country || ', ' || City as Location , Website, Email, 'a' || 55 || ' - ' || ' opa ' as Teste From customers where Country = 'Canada' "  --verbose --bind "customers=customers-100.csv"
#RUN_ARGS= "Select Index, \"First Name\" || ' ' || \"Last Name\" as \"Full Name\" , CURRENT_DATE() as Today, LOWER('TESTE DOIS'), Company, Country || ', ' || City as Location , Website, Email, 'a' || 55 || ' - ' || ' opa ' as Teste From customers where Country = 'Canada' "  --verbose --bind "customers=customers-100.csv"
#RUN_ARGS= "Select Index, \"First Name\" || ' ' || \"Last Name\" as \"Full Name\" , CURRENT_DATE() as Today, LOWER(Company) as \"Company Lowercase\", Company, Country || ', ' || City as Location From customers where Country = 'Canada' "  --verbose --bind "customers=customers-100.csv"
#RUN_ARGS= "Select Index, \"First Name\" || ' ' || \"Last Name\" as \"Full Name\" , Company, Country || ', ' || City as Location From customers where Country = 'Canada' AND Company = 'Wade PLC' "  --verbose --bind "customers=customers-100.csv"
#RUN_ARGS= "Select Index, \"First Name\" || ' ' || \"Last Name\" as \"Full Name\" , UPPER(Company), Country || ', ' || City as Location From customers where Country = 'Canada' AND UPPER(Company) = 'WADE PLC' "  --verbose --bind "customers=customers-100.csv"
#RUN_ARGS= "Describe \"Iris 2\" " --verbose --bind "Iris 2=Iris.csv"
#RUN_ARGS= "Select * From Iris Where Species like '%setosa%' " --verbose --bind "Iris=Iris.csv"
#RUN_ARGS= "Select * From Iris Where Id = 50 and Species like '%setosa%'" --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select * From Iris Where Id < 45 and Species like '%setosa%'" --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select *, CURRENT_DATE() From Iris Where SepalWidthCm > 4 and Species like '%setosa%'" --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select * From Iris Where Id < 45 and Species like '%setosa%' Group By PetalWidthCm   " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select * From Iris Where Id < 45 and Species like '%setosa%' and SepalWidthCm > 4 Group By PetalWidthCm, SepalWidthCm    " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select * From Iris Where Id < 45 and Species like '%setosa%' and SepalWidthCm > 4 Group By PetalWidthCm    " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select Species, Id as IDD, Species  From Iris where Id > 2 and Id < 10 " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select  Species, SepalWidthCm, COUNT(*)  From Iris Group By Species, SepalWidthCm " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select  Species, COUNT(*)  From Iris Group By Species " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select  Species, SepalWidthCm, COUNT(*), MAX(SepalLengthCm)  From Iris Group By Species, SepalWidthCm " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select  Species, SepalWidthCm, COUNT(*), MAX(SepalLengthCm)  From Iris Group By Species, SepalWidthCm " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select  Species, COUNT(*), MAX(SepalLengthCm)  From Iris Group By Species " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select  Species, SepalWidthCm, COUNT(*), MAX(SepalWidthCm)  From Iris Group By Species, SepalWidthCm " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select  Species, SUM(SepalLengthCm), MAX(SepalLengthCm), COUNT(*)  From Iris Where Id < 3 Group By Species " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select  Species, SUM(SepalLengthCm), MIN(SepalLengthCm), MAX(SepalLengthCm), COUNT(*)  From Iris Where Id < 3 Group By Species " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "Select  Species, SUM(SepalLengthCm), MIN(SepalLengthCm), MAX(SepalLengthCm), COUNT(*)  From Iris Group By Species " --verbose --bind "Iris=Iris.csv" #--print-tokens
#RUN_ARGS= "SELECT Series_Title, Released_Year as Year, Certificate as \"Cert\", Runtime, Genre, IMDB_Rating, Meta_score, Director, Gross from imdb_top_1000 " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT Released_Year as Year, COUNT(*), MIN(IMDB_Rating), MAX(IMDB_Rating), AVG(IMDB_Rating) from imdb_top_1000 Where Series_Title like '%%' Group By Released_Year" --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT Released_Year as Year, COUNT(*), FIRST_VALUE(Series_Title) from imdb_top_1000 Group By Released_Year" --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT Series_Title, SUBSTRING(Series_Title, 0, 15 ) from imdb_top_1000 Where Series_Title like '%%' " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT Series_Title, COALESCE(UPPER(SUBSTRING(Series_Title, 0)), 'Default') from imdb_top_1000 Where Series_Title like '%%' " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT COUNT(*), MIN(IMDB_Rating), MAX(IMDB_Rating), AVG(IMDB_Rating) from imdb_top_1000 Where Series_Title like '%%' " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT COUNT(*), MIN(IMDB_Rating), MAX(IMDB_Rating), AVG(IMDB_Rating) from imdb_top_1000 Where Series_Title like '%%' Order By 1 " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT Series_Title, COALESCE(UPPER(SUBSTRING(Series_Title, 0)), 'Default'), IMDB_Rating from imdb_top_1000 Where Series_Title like '%%' Order By 1 Asc " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT Series_Title, AVG(Series_Title) from imdb_top_1000  Group By Series_Title " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT SUBSTRING(Series_Title, 0, 5) AS Title, TO_NUMBER(Series_Title), TO_NUMBER(Series_Title, 0), TO_NUMBER(Series_Title, TO_NUMBER('999999', 88888)), TO_NUMBER(IMDB_Rating), TO_NUMBER(IMDB_Rating, 99999999) from imdb_top_1000 where Series_Title like '%br%' " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT SUM(Released_Year), AVG(Released_Year), MAX(Released_Year), MIN(Released_Year) from imdb_top_1000 " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT SUM(Released_Year), AVG(Released_Year), MAX(Released_Year), MIN(Released_Year), SUM(TO_NUMBER(Released_Year)), AVG(TO_NUMBER(Released_Year)), MAX(TO_NUMBER(Released_Year)), MIN(TO_NUMBER(Released_Year)) from imdb_top_1000 " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT SUM(TO_NUMBER(Released_Year, 0)), AVG(TO_NUMBER(TO_NUMBER(Released_Year, 0))), MAX(TO_NUMBER(Released_Year, 0)), MIN(TO_NUMBER(Released_Year, 9999999)) from imdb_top_1000 " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT TO_NUMBER(Released_Year, 0) from imdb_top_1000 " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT SUM(TO_NUMBER(Released_Year, 0)), AVG(TO_NUMBER(Released_Year)), MAX(TO_NUMBER(Released_Year)), MIN(TO_NUMBER(Released_Year)) from imdb_top_1000 " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT Series_Title, Series_Title,  Series_Title, Series_Title from imdb_top_1000 Order By 1 Asc , 2 Asc , 3 asc, 4 asc	 " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT SUM(TO_NUMBER(Released_Year, 0)) from imdb_top_1000 Order By 1 Asc, 1 Asc" --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "SELECT AVG(IMDB_Rating), AVG(TO_NUMBER(Meta_score, 0)) from imdb_top_1000 " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
RUN_ARGS= "SELECT Series_Title from imdb_top_1000 Where Series_Title like '%Man%' or Series_Title like '%man%'  " --bind "imdb_top_1000=imdb_top_1000.csv" --verbose  --csv-filename imdb_top_1000.csv #--print-tokens


# fazer funcionar

build-folder-setup:
	@ mkdir -p $(BUILD_FOLDER_NAME)

lib/libcsv.a:
	$(MAKE) -C lib

build-libs: lib/libcsv.a

main: build-folder-setup build-libs ./$(SOURCE_FOLDER_NAME)/main.cpp  
	$(CC) ./$(SOURCE_FOLDER_NAME)/main.cpp -o $(BUILD_FOLDER_NAME)/main $(CFLAGS) $(LFLAGS) $(DEF)

tests: build-folder-setup build-libs ./$(TESTS_FOLDER_NAME)/main.cpp  
	$(CC) ./$(TESTS_FOLDER_NAME)/main.cpp -o $(BUILD_FOLDER_NAME)/tests $(CFLAGS) $(LFLAGS) $(DEF)

run: main
	@echo "cd ./$(BUILD_FOLDER_NAME)"
	@echo './main $(RUN_ARGS)'
	@echo "================"
	@cd ./$(BUILD_FOLDER_NAME) && ./main $(RUN_ARGS)
	@echo "\n================\n"

run-tests: tests
	@echo "cd ./$(BUILD_FOLDER_NAME)"
	@echo ./tests
	@echo "================"
	@cd ./$(BUILD_FOLDER_NAME) && ./tests
	@echo "\n================\n"
