
EXECUTABLE := program

CXX := mpic++
CXXFLAGS := -O3 -Wall -Wextra -Werror -Wno-unused-parameter 
LFLAGS :=

SOURCE_FILES := program.cpp support.cpp
OBJ_FILES := $(patsubst %.cpp, %.o, $(SOURCE_FILES))

$(EXECUTABLE): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(OBJ_FILES) -o $@ $(LFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LFLAGS)

clean:
	rm -rf $(EXECUTABLE) *.o *.d

PHONY: clean
	
