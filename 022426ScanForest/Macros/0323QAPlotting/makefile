CXX := g++
TARGET := SaveHistChain
ROOTCFLAGS := $(shell root-config --cflags)
ROOTLIBS   := $(shell root-config --glibs)

CXXFLAGS := -O3 -Wall -Wextra $(ROOTCFLAGS) -I. -I./Include -I./Src

SRC := SaveHistChain.cpp $(wildcard Src/*.cpp)
OBJ := $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(ROOTLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)