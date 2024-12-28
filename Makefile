CC = gcc
CFLAGS = -g
EXEC = cjit
SOURCE = cjit.c src_lexer/lexer.c hashmap/hashmap.c parser/parser.c parser/parser_utils.c parser/casting.c defc/defc.c
TARGET = tests/test1.cj

all: $(EXEC)
	./$(EXEC) $(TARGET)

time: $(EXEC)
	time ./$(EXEC) $(TARGET)

$(EXEC): $(SOURCE)
	$(CC) $(CFLAGS) -o $(EXEC) $(SOURCE)

clean:
	rm -f $(EXEC)

