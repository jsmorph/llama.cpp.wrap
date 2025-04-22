LLAMA_CPP_DIR = ../llama.cpp

all:
	g++ -std=c++17 -O3 -c wrap.cpp -I. -I$(LLAMA_CPP_DIR)/common -I$(LLAMA_CPP_DIR)/ggml/include -I$(LLAMA_CPP_DIR)/include
	g++ -std=c++17 -O3 -c server.cpp -I. -I$(LLAMA_CPP_DIR)/common -I$(LLAMA_CPP_DIR)/ggml/include -I$(LLAMA_CPP_DIR)/include
	g++ -std=c++17 -O3 wrap.o server.o -o llama_server -L$(LLAMA_CPP_DIR)/build/bin -lllama -pthread -fopenmp

example: example.o wrap.o
	g++ -std=c++17 -O3 example.o wrap.o -o example -L$(LLAMA_CPP_DIR)/build/bin -lllama -pthread -fopenmp

example.o: example.c wrap.h
	g++ -std=c++17 -O3 -c example.c -I. -I$(LLAMA_CPP_DIR)/common -I$(LLAMA_CPP_DIR)/ggml/include -I$(LLAMA_CPP_DIR)/include

run:
	LD_LIBRARY_PATH=. ./llama_server models/tinyllama.gguf 8080

test:	test1 test2 test3

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

test-example: example
	echo '{"prompt":"Hello, world!","max_tokens":5,"temperature":0.7,"top_p":0.8,"top_k":30,"seed":42,"include_logits":false}' | \
		LD_LIBRARY_PATH=. ./example models/tinyllama.gguf 128 | tee test-example.json
