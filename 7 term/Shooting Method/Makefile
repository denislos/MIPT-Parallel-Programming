
EXECUTABLE := shooting

CC := mpicc
CFLAGS := -O3 
LFLAGS := -lm

SOURCE_FILES := shooting.c support.c
OBJ_FILES := $(patsubst %.c, %.o, $(SOURCE_FILES))

$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(CLFAGS) $(OBJ_FILES) -o $@ $(LFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

clean:
	rm -rf $(EXECUTABLE) *.o *.d

PHONY: clean
	
