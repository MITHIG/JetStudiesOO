# Set Cling resource directory
CLING_RESOURCE_DIR := $(shell root-config --prefix)/lib/clang/16
export CLING_RESOURCE_DIR

ROOUNFOLD_DIR := /home/data/public/xirong/Packages/RooUnfold/
ROOUNFOLD_LIBS = -L$(ROOUNFOLD_DIR) -lRooUnfold

CXX := g++
TARGET := UnfoldingHalfHalf
ROOTCFLAGS := $(shell root-config --cflags)
ROOTLIBS   := $(shell root-config --glibs)
OPT_FLAGS := -O3
WARN_FLAGS := -Wall -Wextra
LIBS = $(ROOT_LIBS) $(ROOUNFOLD_LIBS)

# Then in your compile rule, add:

INCLUDE_DIRS := -I. \
                -I./Include \
                -I./Src \
                -I../../CommonFunctions/Include \
                -I../../CommonFunctions/Src \
                -I$(ROOUNFOLD_DIR)/src

CXXFLAGS := $(OPT_FLAGS) $(WARN_FLAGS) $(ROOTCFLAGS) $(INCLUDE_DIRS)

# Source and Object files
SRC := UnfoldHalfHalf_rng.cpp $(wilrmdcard Src/*.cpp) $(wildcard ../../CommonFunctions/Src/*.cpp)
OBJ := $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(ROOTLIBS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)