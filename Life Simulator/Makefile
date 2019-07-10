
EXECUTABLE := lisim_perfrunner

CXX := mpic++
CXXFLAGS := -Wall -Wextra -Werror -O3 -std=c++11

SOURCE_FILES := lisimperf.cpp core_matrix.cpp
HEADERS := config.h tools.h core_matrix.h
OBJ_FILES := $(patsubst %.cpp, %.o, $(SOURCE_FILES))

$(EXECUTABLE): $(OBJ_FILES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(OBJ_FILES) -o $@
	#cp $(EXECUTABLE) $(HOME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(EXECUTABLE) *.o *.d

PHONY: clean
	
