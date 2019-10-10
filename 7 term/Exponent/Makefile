
EXECUTABLE := exponent

CC := gcc
CFLAGS := -O3 -Wall -Wextra -Werror
LFLAGS := -fopenmp

SOURCE_FILES := exponent.c support.c
OBJ_FILES := $(patsubst %.c, %.o, $(SOURCE_FILES))

$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $@ $(LFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

clean:
	rm -rf $(EXECUTABLE) *.o *.d

PHONY: clean
	
