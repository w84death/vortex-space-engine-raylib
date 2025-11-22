# Vortex Space Engine - Simplified Makefile

# Compilers
CC = gcc
MINGW_CC = x86_64-w64-mingw32-gcc

# Common flags
CFLAGS = -O3 -march=native -Wall -Wextra -std=c99 -flto -ffast-math

# Source and target
SOURCE = main.c
TARGET = vortex

# Default target: run the program
run: $(TARGET)
	./$(TARGET)

# Build native Linux executable (for development)
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Build static Linux executable
linux-static: $(SOURCE)
	$(CC) $(CFLAGS) -static -o $(TARGET)-linux $(SOURCE) \
		-lraylib -lGL -lm -lpthread -ldl -lrt -lX11 \
		-Wl,--whole-archive -lpthread -Wl,--no-whole-archive

# Build static Windows executable (cross-compile)
windows-static: $(SOURCE)
	$(MINGW_CC) $(CFLAGS) -static -o $(TARGET)-windows.exe $(SOURCE) \
		-lraylib -lopengl32 -lgdi32 -lwinmm

# Build both static versions
build: linux-static windows-static

# Clean all built files
clean:
	rm -f $(TARGET) $(TARGET)-linux $(TARGET)-windows.exe

# Help target
help:
	@echo "Vortex Space Engine - Build Commands"
	@echo "===================================="
	@echo "  make run           - Build and run the program"
	@echo "  make linux-static  - Build static Linux executable"
	@echo "  make windows-static - Build static Windows executable (cross-compile)"
	@echo "  make build         - Build both static versions"
	@echo "  make clean         - Remove all built files"

.PHONY: run linux-static windows-static build clean help
