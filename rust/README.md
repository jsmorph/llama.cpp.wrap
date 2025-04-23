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

## Using as a Library

You can use the high-level API from your own Rust code by adding this crate as a dependency. If using locally, add to your `Cargo.toml`:

```toml
[dependencies]
llama_wrapper = { path = "../llama.cpp.wrap/rust" }
```

### Example

```rust
use llama_wrapper::{LlamaModel, LlamaError};

fn main() -> Result<(), LlamaError> {
    // Set up the model once
    let model_path = "../models/llama-7b.gguf";
    let n_ctx = 512;
    let model = LlamaModel::new(model_path, n_ctx)?;

    // You can now repeatedly get completions for this model
    let request_json = r#"{
        "prompt": "What is the capital of France?",
        "max_tokens": 16,
        "temperature": 0.7,
        "top_p": 0.9,
        "top_k": 40,
        "seed": 42,
        "include_logits": false
    }"#;

    let response_json = model.complete(request_json)?;
    println!("Response: {}", response_json);

    // ...call model.complete(...) as many times as needed

    Ok(())
}
```

- `LlamaModel::new(model_path, n_ctx)`: Loads the model and prepares it for completions.
- `model.complete(request_json)`: Gets a completion for the given JSON request string.

The `complete` method returns a `Result<String, LlamaError>`, where the `String` is the response JSON.

### Requirements

- The llama.cpp C library and wrapper must be built and available for linking.
- The model file must exist at the specified path.
- This crate must be built with the same configuration as the C library.


## Implementation Details

This Rust implementation:

1. Uses FFI to interface with the C functions defined in wrap.h
2. Handles JSON parsing and serialization using serde_json
3. Manages memory safely, ensuring proper cleanup of resources
4. Provides the same functionality as the original C implementation
