# Llama.cpp Rust Wrapper

This is a Rust implementation of the example.c program for interacting with the llama.cpp library.

## Prerequisites

- Rust and Cargo installed
- The llama.cpp library and wrapper compiled (in the parent directory)

## Building

To build the project, follow these steps:

1. First, build the static library:

```bash
cd rust
make
```

2. Then build the Rust program:

```bash
cargo build --release
```

This will compile the Rust program and link it with the necessary libraries.

## Usage

The program usage is identical to the C version:

```bash
./target/release/llama_wrapper <model_path> <context_size>
```

Example:

```bash
./target/release/llama_wrapper ../models/llama-7b.gguf 2048
```

The program reads JSON completion requests from stdin, processes them using the llama.cpp library, and outputs the results as JSON to stdout.

Example input:

```json
{"prompt": "What is the capital of France?", "max_tokens": 100, "temperature": 0.7, "top_p": 0.9, "top_k": 40, "seed": 42, "include_logits": false}
```

## Implementation Details

This Rust implementation:

1. Uses FFI to interface with the C functions defined in wrap.h
2. Handles JSON parsing and serialization using serde_json
3. Manages memory safely, ensuring proper cleanup of resources
4. Provides the same functionality as the original C implementation
