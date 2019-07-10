
EXECUTABLE := lab1_solution

CC := mpicc
CFLAGS := -Wall -Wextra -Werror -O3
LFLAGS := -lm

SOURCE_FILES := solution.c
HEADERS := problem_config.h tools.h
OBJ_FILES := $(patsubst %.c, %.o, $(SOURCE_FILES))

$(EXECUTABLE): $(OBJ_FILES) $(HEADERS)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $@ $(LFLAGS)
	#cp $(EXECUTABLE) $(HOME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXECUTABLE) *.o *.d

PHONY: clean
	
