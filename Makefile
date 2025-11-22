CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O3 -march=native -flto -ffast-math
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

TARGET = terrain_renderer
SOURCE = main.c

# Default target
all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
