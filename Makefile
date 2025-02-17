CC = g++

## agregar flags? como -g o -Wall
CFLAGS =

## agregar librerias extra
LDLIBS=

TARGET=./build/main
TEST=./build/test

## AQUI INCLUIR LOS ARCHIVOS .C 
## excepto main y test
## agregar un "\" al final de cada nombre
SRC = \
	  src/mtree/point.cpp\
	  src/mtree/mtree.cpp\
	  src/mtree/mtree_create_cp.cpp\
	  src/mtree/mtree_create_ss.cpp\
	  src/utils/random.cpp\
	  src/utils/closest_pair.cpp


## nombre de los headers (automatico)
HDRS = $(wildcard headers/*.hpp)
## nombre de los .o (automatico)
OBJS = $(SRC:./src/%.cpp=./build/%.o)



all:
	@echo " MTree implementation proyect"
	@echo ""
	@echo " use \"make help\" to get more info"

## Compila un .o a partir del src
./build/%.o: ./src/*/%.cpp $(HDRS)
	@ $(CC) $(CFLAGS)  -c $< -o $@

## Compila un .o a partir del src
./build/%.o: ./src/%.cpp $(HDRS)
	@ $(CC) $(CFLAGS)  -c $< -o $@

## Compila el ejecutable del main y ejecuta
run: $(OBJS) $(TARGET).o
	@ $(CC) $(CFLAGS) $^ -o $(TARGET) $(LDLIBS) && $(TARGET)

## Compila el ejecutable de los test y ejecuta
test: $(OBJS) $(TEST).o
	@ $(CC) $(CFLAGS) $^ -o $(TEST) $(LDLIBS) && $(TEST)

## borra todo lo que se compilo
clean: $(wildcard build/*.o)
	@ rm -f $(TARGET)
	@ rm -f $(TEST)
	@ rm -f $^


help:
	@echo " to run main --> make run"
	@echo " to run test --> make test"
	

