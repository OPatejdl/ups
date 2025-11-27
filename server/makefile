CPP = g++
CPPFLAGS = -Wall -Wextra -I.

# output file
BIN = server.exe

OBJ = main.o \
	exceptions/exceptions.o \
	logger/logger.o \
	utility/utility.o \
	userHandling/user.o \
	userHandling/userManager.o \
	server/server.o

# compilation
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

# final build
$(BIN): $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@

# remove
clean:
	rm -f $(OBJ) $(BIN)