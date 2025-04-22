# llama.cpp.wrap

_Gemini 2.5 Pro wrote all of this code and documentation_

Minimal HTTP wrapper for llama.cpp inference.

## Overview

This project provides a lightweight, efficient HTTP wrapper around llama.cpp, enabling easy access to large language model inference capabilities through a simple API. The wrapper is designed to be minimal yet functional, focusing on performance and ease of integration.

Key components:

- `wrap.cpp`: C++ wrapper for llama.cpp. Loads a model, runs completions, and manages resources. Provides a clean C interface that abstracts away the complexity of direct llama.cpp usage.
- `server.cpp`: HTTP server exposing a `/complete` endpoint for text generation and a `/health` endpoint. Built with the lightweight httplib library for minimal dependencies.
- `example.c`: Demonstrates direct usage of the wrapper API through a command-line JSON interface.

Architecture:
- The wrapper layer (`wrap.cpp`/`wrap.h`) handles all direct interaction with llama.cpp, including model loading, token generation, and resource management
- The server layer (`server.cpp`) provides HTTP access to the wrapper functionality, handling request parsing, response formatting, and error handling
- Both components are designed to be statically linked for easy deployment without dependency issues

This design allows for flexible usage patterns - either through the HTTP API for networked applications or direct C API integration for embedded use cases.

## Build

Requires `g++`, `make`, and llama.cpp.

This project now supports static linking with llama.cpp. The Makefile will automatically build the static libraries if they don't exist.

```sh
make
```

The build process:
1. Compiles the C++ wrapper and server code
2. Builds the llama.cpp static libraries if needed
3. Links everything together into statically-linked executables

No need to set `LD_LIBRARY_PATH` anymore - the executables include all necessary code.

## Run

```sh
make run
```
This runs the server on port 8080 with the model at `models/tinyllama.gguf`. Adjust as needed.

## API

### POST `/complete`

Request (JSON):

```json
{
  "prompt": "string",                // required
  "max_tokens": 128,                 // optional, default 128
  "temperature": 0.8,                // optional, default 0.8
  "top_p": 0.9,                      // optional, default 0.9
  "top_k": 40,                       // optional, default 40
  "seed": -1,                        // optional, default -1 (random)
  "include_logits": false            // optional, default false
}
```

Response (JSON):

```json
{
  "text": "string",                  // generated text
  "seed": 42,                        // seed used
  "tokens": [                        // present if include_logits=true
    { "id": 123, "text": "foo", "logit": -1.23 }
  ]
}
```

- Content-Type must be `application/json`.
- Returns HTTP 400 for invalid input, 500 for internal errors.

### GET `/health`

Returns:

```json
{"status":"ok"}
```

## Network Protocol Version

- No explicit versioning. The API is stable as described above.

## Test

Start the server, then run:

```sh
make test
```

This runs three example POST requests to `/complete` and saves the outputs as `test1.json`, `test2.json`, and `test3.json`.

## C Example: Command-Line JSON Completion

A minimal C program, `example.c`, is provided to demonstrate direct use of the C API for JSON-based completion.

### Build

```sh
make example
```

### Usage

```sh
./example <model_path> <context_size>
```

- `<model_path>`: Path to a GGUF model file (e.g., `models/tinyllama.gguf`)
- `<context_size>`: Context window size (e.g., 128)

The program reads lines of JSON from standard input. Each line must be a valid completion request object:

```json
{"prompt":"Hello, world!","max_tokens":5,"temperature":0.7,"top_p":0.8,"top_k":30,"seed":42,"include_logits":false}
```

For each line, the program:
1. Parses the JSON as a completion request.
2. Runs the model to generate a response.
3. Serializes the response as JSON and writes it to standard output.

### Example

You can test the example program with:

```sh
make test-example
```

This will run `example` with a sample request and save the output to `test-example.json`.

Sample output:

```json
{"text":"\nDear Sir/","tokens":null}
```
