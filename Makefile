CC = gcc
CFLAGS = -g
EXEC = cjit
SOURCE = cjit.c src_lexer/lexer.c hashmap/hashmap.c
TARGET = tests/test1.cj

all: $(EXEC)
	./$(EXEC) $(TARGET)

$(EXEC): $(SOURCE)
	$(CC) $(CFLAGS) -o $(EXEC) $(SOURCE)

clean:
	rm -f $(EXEC)

