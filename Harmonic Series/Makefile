
EXECUTABLE := harmonic

CC := mpicc
CFLAGS := -Wall -Werror -Wextra
MPI_RUNNER := mpiexec 

SOURCE_FILES := harmonic.c
OBJ_FILES := $(patsubst %.c, %.o, $(SOURCE_FILES))

$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $@

simple_run: $(EXECUTABLE)
	$(MPI_RUNNER) $(EXECUTABLE)

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -rf $(EXECUTABLE) *.o *.d

PHONY: clean
	