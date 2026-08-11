# Linux-Build: make 
# Windows-Build: make win
# Beider: make both


# ================================
# Compiler und Flags
# ================================
LINUX_CC := gcc
WIN_CC := x86_64-w64-mingw32-gcc

CFLAGS := -Wall -Wextra -Iinclude

# ================================
# Ordner
# ================================
SRC_DIR := src
BUILD_LINUX := build
BUILD_WIN := build_win

# ================================
# Dateien
# ================================
SRC := $(wildcard $(SRC_DIR)/*.c)

OBJ_LINUX := $(patsubst $(SRC_DIR)/%.c, $(BUILD_LINUX)/%.o, $(SRC))
OBJ_WIN := $(patsubst $(SRC_DIR)/%.c, $(BUILD_WIN)/%.o, $(SRC))

TARGET_LINUX := $(BUILD_LINUX)/main
TARGET_WIN := $(BUILD_WIN)/main.exe

# ================================
# Default: Linux
# ================================
all: linux
both: linux win

# ================================
# Linux build
# ================================
linux: $(TARGET_LINUX)

$(TARGET_LINUX): $(OBJ_LINUX)
	@mkdir -p $(BUILD_LINUX)
	$(LINUX_CC) $(CFLAGS) -o $@ $^

$(BUILD_LINUX)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_LINUX)
	$(LINUX_CC) $(CFLAGS) -c $< -o $@

# ================================
# Windows build
# ================================
win: $(TARGET_WIN)

$(TARGET_WIN): $(OBJ_WIN)
	@mkdir -p $(BUILD_WIN)
	$(WIN_CC) $(CFLAGS) -o $@ $^

$(BUILD_WIN)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_WIN)
	$(WIN_CC) $(CFLAGS) -c $< -o $@

# ================================
# Aufräumen
# ================================
clean:
	rm -rf $(BUILD_LINUX) $(BUILD_WIN)

# ================================
# Direkt ausführen
# ================================
run: linux
	./$(TARGET_LINUX)
