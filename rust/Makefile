PARENT_DIR = ..
LLAMA_CPP_DIR = ../../llama.cpp
LLAMA_BUILD_DIR = $(LLAMA_CPP_DIR)/build-static
LLAMA_LIB_DIR = $(LLAMA_BUILD_DIR)/src
GGML_LIB_DIR = $(LLAMA_BUILD_DIR)/ggml/src

# Compiler flags
CXX = g++
CXXFLAGS = -std=c++17 -O3
INCLUDES = -I$(PARENT_DIR) -I$(LLAMA_CPP_DIR)/common -I$(LLAMA_CPP_DIR)/ggml/include -I$(LLAMA_CPP_DIR)/include
LDFLAGS = -pthread -fopenmp

# Default target
all: libwrap.a

# Compile wrap.o
wrap.o: $(PARENT_DIR)/wrap.cpp $(PARENT_DIR)/wrap.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Create static library
libwrap.a: wrap.o
	ar rcs $@ $<

# Clean target
clean:
	rm -f *.o *.a

.PHONY: all clean
