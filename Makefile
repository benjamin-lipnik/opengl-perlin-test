
BIN=Program
CC=clang
FLAGS=$$(pkg-config --cflags glfw3) $$(pkg-config --static --libs glfw3) -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lGLEW -lm
OBJS=main.o

all: $(BIN)

%.o: %.c %.h
	$(CC) $(FLAGS) -c $^

$(BIN): main.o shader_loader.o bMatrix.o 
	$(CC) $(FLAGS) -o $@ $^

run: $(BIN)
	@echo 
	@echo ----------------------
	@echo 
	@./Program
	@echo 

clean:
	rm *.o $(BIN)
