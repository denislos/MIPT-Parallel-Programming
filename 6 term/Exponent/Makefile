
EXECUTABLE := exponent

CC := mpicc
CFLAGS := -Wall -Wextra -w
DFLAGS := -DUSE_GMP
LFLAGS := -lgmp
MPI_RUNNER := mpiexec 

SOURCE_FILES := exponent.c mpi_gmp.c
OBJ_FILES := $(patsubst %.c, %.o, $(SOURCE_FILES))

$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $@ $(LFLAGS)

simple_run: $(EXECUTABLE)
	$(MPI_RUNNER) $(EXECUTABLE)

%.o: %.c
	$(CC) $(CFLAGS) $(DFLAGS) -c $< -o $@ $(LFLAGS)

clean:
	rm -rf $(EXECUTABLE) *.o *.d

PHONY: clean
	
