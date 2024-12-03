# Compiler and flags
CC = clang # whatever compiler you use
CFLAGS = -I/opt/homebrew/include #add abs path to where the include folder of libpng is
LDFLAGS = -L/opt/homebrew/lib -lpng16 -lz #add abs path to where libpng is
#for me they where at this location yours might be different


# Target executable
TARGET = Image_process

# Source file
SRC = Image_process.c ReadWrite.c

# Build target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(SRC)

# Clean up build files
clean:
	rm -f $(TARGET)