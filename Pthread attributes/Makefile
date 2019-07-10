
EXECUTABLE := print_attr

CC := gcc
CFLAGS := -O0 -w
LFLAGS := -lpthread

SOURCE_FILES := print_attr.c
OBJ_FILES := $(patsubst %.c, %.o, $(SOURCE_FILES))

$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $@ $(LFLAGS)
	#cp $(EXECUTABLE) $(HOME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXECUTABLE) *.o *.d

PHONY: clean
	
