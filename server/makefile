CPP = g++
CPPFLAGS = -Wall -Wextra -I. -std=c++17

# dir setup
OBJ_DIR = obj
BIN_DIR = bin

# output file
BIN = $(BIN_DIR)/server.exe

SRCS = src/main.cpp \
	src/exceptions/exceptions.cpp \
	src/logger/logger.cpp \
	src/utility/utility.cpp \
	src/userHandling/user.cpp \
	src/userHandling/userManager.cpp \
	src/server/server.cpp

OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(BIN)

# compilation
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CPP) -c $(CPPFLAGS) $< -o $@

# final build
$(BIN): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CPP) $(CPPFLAGS) $^ -o $@

# remove
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# info
info:
	@echo "Sources: $(SRCS)"
	@echo "Objects: $(OBJ)"
	@echo "Exe: $(BIN)"