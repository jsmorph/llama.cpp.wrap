# LlamaGo

A Go wrapper for llama.cpp, providing similar functionality to the C example.

## Prerequisites

- Go 1.21 or later
- GCC or another C compiler compatible with CGO
- llama.cpp built in the parent directory

## Building

To build the project, run:

```bash
make
```

This will compile the Go code and link it with the necessary C libraries.

## Testing

To run a simple test with a predefined prompt, use:

```bash
make test MODEL_PATH=../models/llama-7b.gguf
```

Replace `../models/llama-7b.gguf` with the path to your model file.

## Usage

The usage is similar to the C example:

```bash
./llamago <model_path> <context_size>
```

Example:

```bash
./llamago ../models/llama-7b.gguf 2048
```

The program reads JSON-formatted completion requests from standard input, one per line, and writes JSON-formatted completion results to standard output.

Example input:

```json
{"prompt": "Hello, I am", "max_tokens": 50, "temperature": 0.7, "top_p": 0.9, "top_k": 40, "seed": 42, "include_logits": false}
```

## Implementation Details

This Go implementation uses CGO to interface with the C functions defined in `wrap.h` and implemented in `wrap.cpp`. It provides the same functionality as the C example but with Go's memory management and error handling.
