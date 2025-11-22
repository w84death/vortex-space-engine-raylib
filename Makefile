CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O3 -march=native -flto -ffast-math
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

TARGET = terrain_renderer
SOURCE = main.c

# Windows cross-compiler
MINGW_CC = x86_64-w64-mingw32-gcc

# Platform-specific libraries
LIBS_LINUX = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
LIBS_WINDOWS = -lraylib -lopengl32 -lgdi32 -lwinmm

# Detect OS for native compilation
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LIBS = $(LIBS_LINUX)
endif
ifeq ($(UNAME_S),Darwin)
    LIBS = $(LIBS_LINUX)
endif
ifeq ($(OS),Windows_NT)
    LIBS = $(LIBS_WINDOWS)
    TARGET := $(TARGET).exe
endif

# UPX compression tool
UPX = upx

# Default target
all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)

# Download pre-compiled RayLib for Windows (MinGW)
download-raylib-windows:
	@echo "Downloading RayLib for Windows MinGW..."
	@mkdir -p lib/windows
	@cd lib/windows && \
		wget -q --show-progress https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip && \
		unzip -q raylib-5.5_win64_mingw-w64.zip && \
		echo "RayLib for Windows downloaded successfully!"

# Download pre-compiled RayLib for Linux
download-raylib-linux:
	@echo "Downloading RayLib for Linux..."
	@mkdir -p lib/linux
	@cd lib/linux && \
		wget -q --show-progress https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_linux_amd64.tar.gz && \
		tar -xzf raylib-5.5_linux_amd64.tar.gz && \
		echo "RayLib for Linux downloaded successfully!"

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

# Linux compilation with fully static linking (maximum portability)
linux-static: download-raylib-linux
	@command -v $(UPX) >/dev/null 2>&1 || { \
		echo "UPX is not installed. Install it to compress executables."; \
		echo "  Ubuntu/Debian: sudo apt-get install upx-ucl"; \
		echo "  Fedora: sudo dnf install upx"; \
		echo "  Arch: sudo pacman -S upx"; \
		exit 1; \
	}
	$(CC) $(SOURCE) -o $(TARGET)-static -O3 -DNDEBUG -Wall -Wextra -std=c99 \
		-I./lib/linux/raylib-5.5_linux_amd64/include \
		-L./lib/linux/raylib-5.5_linux_amd64/lib \
		-static -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 \
		-Wl,--whole-archive -lpthread -Wl,--no-whole-archive
	$(UPX) --best --lzma $(TARGET)-static

# Release builds for all platforms
release: clean linux windows

# Create Linux distribution package
dist-linux: linux
	mkdir -p dist
	cp $(TARGET) dist/
	echo "Terrain Renderer - Vortex Space Engine (Standalone Linux)" > dist/VERSION.txt
	echo "Version: 1.0.0" >> dist/VERSION.txt
	echo "Build Date: $(shell date)" >> dist/VERSION.txt
	tar -czf terrain-renderer-linux-$(shell date +%Y%m%d).tar.gz dist/
	rm -rf dist/
	echo "Standalone Linux distribution package created: terrain-renderer-linux-$(shell date +%Y%m%d).tar.gz"

# Create static Linux distribution package
dist-linux-static: linux-static
	mkdir -p dist
	cp $(TARGET)-static dist/$(TARGET)
	echo "Terrain Renderer - Vortex Space Engine (Fully Static Linux)" > dist/VERSION.txt
	echo "Version: 1.0.0" >> dist/VERSION.txt
	echo "Build Date: $(shell date)" >> dist/VERSION.txt
	tar -czf terrain-renderer-linux-static-$(shell date +%Y%m%d).tar.gz dist/
	rm -rf dist/
	echo "Fully static Linux distribution package created: terrain-renderer-linux-static-$(shell date +%Y%m%d).tar.gz"

# Create Windows distribution package
dist-windows: windows
	mkdir -p dist-win
	cp $(TARGET).exe dist-win/
	echo "@echo off" > dist-win/run.bat
	echo "$(TARGET).exe" >> dist-win/run.bat
	echo "pause" >> dist-win/run.bat
	zip -r terrain-renderer-win-$(shell date +%Y%m%d).zip dist-win/
	rm -rf dist-win/
	echo "Windows distribution package created: terrain-renderer-win-$(shell date +%Y%m%d).zip"

clean:
	rm -f $(TARGET) $(TARGET).exe $(TARGET)-static
	rm -rf lib/ dist/ dist-win/

run: $(TARGET)
	./$(TARGET)

# Clean and rebuild
rebuild: clean all

# Help
help:
	@echo "Terrain Renderer - Vortex Space Engine Makefile"
	@echo "================================================"
	@echo ""
	@echo "Building:"
	@echo "  make              - Build for current system"
	@echo "  make run          - Build and run the program"
	@echo "  make clean        - Remove all built files"
	@echo "  make rebuild      - Clean and rebuild"
	@echo ""
	@echo "Cross-Platform Builds (Optimized + UPX Compressed):"
	@echo "  make linux        - Build portable Linux binary with bundled RayLib"
	@echo "  make linux-static - Build fully static Linux binary (maximum portability)"
	@echo "  make windows      - Build Windows exe with bundled RayLib"
	@echo "  make release      - Build both Linux and Windows binaries"
	@echo ""
	@echo "Distribution Packages:"
	@echo "  make dist-linux        - Create Linux distribution package"
	@echo "  make dist-linux-static - Create fully static Linux distribution package"
	@echo "  make dist-windows      - Create Windows distribution package"
	@echo ""
	@echo "Info:"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - gcc (for Linux builds)"
	@echo "  - mingw-w64 (for Windows cross-compilation)"
	@echo "  - upx (for compression)"
	@echo "  - wget, unzip, tar (for downloading RayLib)"

.PHONY: all clean run windows linux linux-static release dist-linux dist-linux-static dist-windows \
        rebuild help download-raylib-windows download-raylib-linux
