CPP = g++
CPPFLAGS = -Wall -Wextra -I.

# output file
BIN = main.exe

OBJ = main.o logger/logger.o utility/utility.o

# compilation
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

# final build
$(BIN): $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@

# remove
clean:
	rm -f $(OBJ) $(BIN)