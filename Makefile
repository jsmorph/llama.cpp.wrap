LLAMA_CPP_DIR = ../llama.cpp
LLAMA_BUILD_DIR = $(LLAMA_CPP_DIR)/build-static
LLAMA_LIB_DIR = $(LLAMA_BUILD_DIR)/src
GGML_LIB_DIR = $(LLAMA_BUILD_DIR)/ggml/src

# Compiler flags
CXX = g++
CXXFLAGS = -std=c++17 -O3
INCLUDES = -I. -I$(LLAMA_CPP_DIR)/common -I$(LLAMA_CPP_DIR)/ggml/include -I$(LLAMA_CPP_DIR)/include
LDFLAGS = -pthread -fopenmp

# Default target
all: llama_server

# Build the static libraries if they don't exist
$(LLAMA_LIB_DIR)/libllama.a $(GGML_LIB_DIR)/libggml.a $(GGML_LIB_DIR)/libggml-base.a $(GGML_LIB_DIR)/libggml-cpu.a:
	cmake -DBUILD_SHARED_LIBS=OFF $(LLAMA_CPP_DIR) -B $(LLAMA_BUILD_DIR)
	make -C $(LLAMA_BUILD_DIR)

# Ensure the static libraries are built
build_static_libs:
	cmake -DBUILD_SHARED_LIBS=OFF $(LLAMA_CPP_DIR) -B $(LLAMA_BUILD_DIR)
	make -C $(LLAMA_BUILD_DIR)

# Compile object files
wrap.o: wrap.cpp wrap.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

server.o: server.cpp wrap.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

example.o: example.c wrap.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link executables
llama_server: server.o wrap.o build_static_libs
	$(CXX) $(CXXFLAGS) server.o wrap.o -o $@ $(LDFLAGS) \
		-L$(LLAMA_LIB_DIR) -L$(GGML_LIB_DIR) \
		$(LLAMA_LIB_DIR)/libllama.a $(GGML_LIB_DIR)/libggml.a $(GGML_LIB_DIR)/libggml-base.a $(GGML_LIB_DIR)/libggml-cpu.a

example: example.o wrap.o build_static_libs
	$(CXX) $(CXXFLAGS) example.o wrap.o -o $@ $(LDFLAGS) \
		-L$(LLAMA_LIB_DIR) -L$(GGML_LIB_DIR) \
		$(LLAMA_LIB_DIR)/libllama.a $(GGML_LIB_DIR)/libggml.a $(GGML_LIB_DIR)/libggml-base.a $(GGML_LIB_DIR)/libggml-cpu.a

# Run the server
run: llama_server
	./llama_server models/tinyllama.gguf 8080

# Test targets
test: test1 test2 test3

test1:
	curl -s -X POST "http://localhost:8080/complete" \
		-H "Content-Type: application/json" \
		-d '{"prompt":"What is your favorite color?", "max_tokens":10, "seed":42}' | tee test1.json

test2:
	curl -s -X POST "http://localhost:8080/complete" \
		-H "Content-Type: application/json" \
		-d '{"prompt":"What is your favorite color?", "max_tokens":10, "seed":421}' | tee test2.json

test3:
	curl -s -X POST "http://localhost:8080/complete" \
		-H "Content-Type: application/json" \
		-d '{"prompt":"Tell me a short story", "max_tokens":50, "temperature":0.7, "top_p":0.8, "top_k":30, "seed":123, "include_logits":true}' \
		| tee test3.json | jq -c '.tokens[0:10]'

test-example:
	echo '{"prompt":"What is a funny name of a type of bird?  Just respond with the name. Name:","max_tokens":20,"temperature":0.7,"top_p":0.8,"top_k":30,"seed":42,"include_logits":false}' | \
		./example models/tinyllama.gguf 128 | tee test-example.json

# Clean target
clean:
	rm -f *.o llama_server example

.PHONY: all run test test1 test2 test3 test-example clean
