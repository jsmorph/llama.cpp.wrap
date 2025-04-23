# Makefile for llamago

# Go compiler
GO := go

# C compiler for CGO
CC := gcc

# Build flags
CFLAGS := -I.. -I../../llama.cpp/ggml/include/ggml
LDFLAGS := -L.. -L../../llama.cpp/build-static/ggml/src -Wl,-Bstatic -lwrap -lllama -lggml -lggml-base -lggml-cpu -Wl,-Bdynamic -lm -pthread -lgomp -lstdc++

# Output binary
BIN := llamago

.PHONY: all clean test

all: $(BIN)

$(BIN): main.go
	CGO_ENABLED=1 CGO_CFLAGS="$(CFLAGS)" CGO_LDFLAGS="$(LDFLAGS)" $(GO) build -o $(BIN) .

test: $(BIN)
	@echo '{"prompt": "Hello, I am", "max_tokens": 50, "temperature": 0.7, "top_p": 0.9, "top_k": 40, "seed": 42, "include_logits": false}' | ./$(BIN) $(MODEL_PATH) 2048

clean:
	rm -f $(BIN)
