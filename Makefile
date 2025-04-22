all:
	cp ../build/bin/libllama.so .
	g++ -std=c++17 -O3 -c wrap.cpp -I. -I../common -I../ggml/include -I../include
	g++ -std=c++17 -O3 -c server.cpp -I. -I../common -I../ggml/include -I../include
	g++ -std=c++17 -O3 wrap.o server.o -o llama_server -L../build/bin -lllama -pthread -fopenmp

run:
	LD_LIBRARY_PATH=. ./llama_server models/tinyllama.gguf 8080

test-json:
	curl -X POST "http://localhost:8080/complete" \
		-H "Content-Type: application/json" \
		-d '{"prompt":"Hello", "max_tokens":10, "seed":42}' -m 120

test2:
	curl -X POST "http://localhost:8080/complete" \
		-H "Content-Type: application/json" \
		-d '{"prompt":"Hello", "max_tokens":10, "seed":142}' -m 120

test-all-params:
	curl -X POST "http://localhost:8080/complete" \
		-H "Content-Type: application/json" \
		-d '{"prompt":"Tell me a short story", "max_tokens":50, "temperature":0.7, "top_p":0.8, "top_k":30, "seed":123, "include_logits":true}' -m 120 \
	tee response.json | jq .
