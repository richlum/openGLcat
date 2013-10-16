# OS X
ifeq ("$(shell uname)", "Darwin")
  CXXFLAGS = -I/usr/include
  LIB = -framework OpenGL -framework GLUT

# Linux 
else
  CXXFLAGS = -I/usr/include 
  LIB = -lGLU -lGL -lglut

endif



BIN=a2
OBJ=$(BIN).o kitten.o utils.o


default: build
	
build: $(BIN)

test: build
	./$(BIN)

%.o: %.cpp
	g++ -c -g $(CXXFLAGS) -o $@ $<

$(BIN): $(OBJ)
	g++ -g -o $@ $(LIB) $(OBJ)

clean:
	- rm -f $(BIN) $(OBJ)
