# Compiler and flags
CC = clang
CFLAGS = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lpng16 -lz

# Target executable
TARGET = functions

# Source file
SRC = functions.c

# Build target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(SRC)

# Clean up build files
clean:
	rm -f $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET) img1.png output.png