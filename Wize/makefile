# Compiler and flags
CXX := g++
CXXFLAGS := -Ofast -pthread -DUSE_SSE41 -msse4.1 -DUSE_SSSE3 -mssse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse

# Source files directory
SRC_DIR := src

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
NNUE_SRCS := $(SRC_DIR)/nnue/nnue.cpp $(SRC_DIR)/nnue/misc.cpp
TINYCTHREAD_SRC := $(SRC_DIR)/tinycthread.c

# Output binary name
NAME := Wize

all: $(NAME)

$(NAME): $(SRCS) $(NNUE_SRCS) $(TINYCTHREAD_SRC)
	$(CXX) $(CXXFLAGS) $(SRCS) $(NNUE_SRCS) $(TINYCTHREAD_SRC) -o $(NAME)

clean:
	rm -f $(NAME)
