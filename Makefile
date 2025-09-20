# Configurações de compilação
## usando g++ não precisa do parâmetro -lstdc++
CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++2a
LFLAGS = -Llib -lcsv
DEF = -DDEV_CODE_ENABLED

# Configurações gerais
SOURCE_FOLDER_NAME=src
TESTS_FOLDER_NAME=tests
BUILD_FOLDER_NAME=target

#RUN_ARGS= "SELECT * FROM DUAL" --verbose
#RUN_ARGS= "Select Name, Phone1 , Phone2 From customers Where Name = 'Marcelson' or Name = 'Jucelson' or 'Brunelson' = Name " --verbose #--print-tokens
#RUN_ARGS= "Select Name, Phone1 , Phone2 From customers Where Name = 'Marcelson' OR Name = 'Jucelson' " --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
RUN_ARGS= "Select Poster_Link,Released_Year,Certificate,Genre,IMDB_Rating,Overview,Meta_score,Director,Star1,Star2,Star3,Star4,No_of_Votes,Gross From customers " --verbose  --csv-filename imdb_top_1000.csv #--print-tokens
#RUN_ARGS= "Select Name, Phone1 , Phone2 From customers Where Name = 02" --verbose --print-tokens
#RUN_ARGS= "Select * From customers " --verbose

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
