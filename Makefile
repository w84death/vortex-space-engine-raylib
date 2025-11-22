# Vortex Space Engine - Makefile

# Compilers and Tools
CC = gcc
MINGW_CC = x86_64-w64-mingw32-gcc
UPX = upx

# Source and Target
SOURCE = game.c engine.c terrain.c renderer.c ui.c input.c entities.c
TARGET = game_engine_demo

# Default target: run the program (dev mode using system libraries)
run: $(SOURCE)
	$(CC) -O3 -march=native -Wall -Wextra -std=c99 -flto -ffast-math -o $(TARGET) $(SOURCE) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
	./$(TARGET)

# Create directories for libraries
prep-dirs:
	@mkdir -p lib/windows
	@mkdir -p lib/linux

# Download Raylib for Windows (MinGW)
download-raylib-windows: prep-dirs
	@if [ ! -d "lib/windows/raylib-5.5_win64_mingw-w64" ]; then \
		echo "Downloading Raylib 5.5 for Windows..."; \
		wget -q --show-progress https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip -O lib/windows/raylib-win.zip; \
		echo "Extracting..."; \
		unzip -q lib/windows/raylib-win.zip -d lib/windows/; \
		rm lib/windows/raylib-win.zip; \
		echo "Done."; \
	fi

# Download Raylib for Linux (AMD64)
download-raylib-linux: prep-dirs
	@if [ ! -d "lib/linux/raylib-5.5_linux_amd64" ]; then \
		echo "Downloading Raylib 5.5 for Linux..."; \
		wget -q --show-progress https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_linux_amd64.tar.gz -O lib/linux/raylib-linux.tar.gz; \
		echo "Extracting..."; \
		tar -xzf lib/linux/raylib-linux.tar.gz -C lib/linux/; \
		rm lib/linux/raylib-linux.tar.gz; \
		echo "Done."; \
	fi

# Windows compilation with downloaded RayLib
windows: download-raylib-windows
	@command -v $(UPX) >/dev/null 2>&1 || { \
		echo "UPX is not installed. Install it to compress executables."; \
		echo "  Ubuntu/Debian: sudo apt-get install upx-ucl"; \
		echo "  Fedora: sudo dnf install upx"; \
		echo "  Arch: sudo pacman -S upx"; \
		exit 1; \
	}
	$(MINGW_CC) $(SOURCE) -o $(TARGET).exe -O3 -DNDEBUG -Wall -Wextra -std=c99 \
		-I./lib/windows/raylib-5.5_win64_mingw-w64/include \
		-L./lib/windows/raylib-5.5_win64_mingw-w64/lib \
		-lraylib -lopengl32 -lgdi32 -lwinmm -static
	$(UPX) --best --lzma $(TARGET).exe

# Linux compilation with downloaded RayLib (standalone)
linux: download-raylib-linux
	@command -v $(UPX) >/dev/null 2>&1 || { \
		echo "UPX is not installed. Install it to compress executables."; \
		echo "  Ubuntu/Debian: sudo apt-get install upx-ucl"; \
		echo "  Fedora: sudo dnf install upx"; \
		echo "  Arch: sudo pacman -S upx"; \
		exit 1; \
	}
	$(CC) $(SOURCE) -o $(TARGET) -O3 -DNDEBUG -Wall -Wextra -std=c99 \
		-I./lib/linux/raylib-5.5_linux_amd64/include \
		-L./lib/linux/raylib-5.5_linux_amd64/lib \
		-Wl,-Bstatic -lraylib -Wl,-Bdynamic \
		-lGL -lm -lpthread -ldl -lrt -lX11 \
		-static-libgcc -static-libstdc++
	$(UPX) --best --lzma $(TARGET)

# Clean built files
clean:
	rm -f $(TARGET) $(TARGET).exe

# Clean libraries
clean-libs:
	rm -rf lib/windows lib/linux

.PHONY: run prep-dirs download-raylib-windows download-raylib-linux windows linux clean clean-libs
