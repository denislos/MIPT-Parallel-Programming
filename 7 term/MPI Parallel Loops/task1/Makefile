
EXECUTABLE := task1

CC := mpicc
CFLAGS := -O3 -Wall -Wextra -Werror
LFLAGS := -lm

SOURCE_FILES := task1.c
OBJ_FILES := $(patsubst %.c, %.o, $(SOURCE_FILES))

$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $@ $(LFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

clean:
	rm -rf $(EXECUTABLE) *.o *.d

PHONY: clean
	
