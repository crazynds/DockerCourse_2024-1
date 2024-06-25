# Nome do executável
TARGET = simple_web_server

# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -O2

# Arquivos fonte
SRCS = programa.c

# Arquivos objeto
OBJS = $(SRCS:.c=.o)

# Regras
all: $(TARGET)
	./$(TARGET)

# Regra para criar o executável
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Regra para criar arquivos objeto
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar arquivos objeto e o executável
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean run
