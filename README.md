# llama.cpp.wrap

_Gemini 2.5 Pro wrote all of this code and documentation_

Minimal HTTP wrapper for llama.cpp inference.

## Overview

- `wrap.cpp`: C++ wrapper for llama.cpp. Loads a model, runs completions, and manages resources.
- `server.cpp`: HTTP server exposing a `/complete` endpoint for text generation and a `/health` endpoint.

## Build

Requires `g++`, `make`, and llama.cpp.

You must build llama.cpp as a shared library (`libllama.so`). Clone https://github.com/ggerganov/llama.cpp, then run:

```sh
cd ../llama.cpp
cmake -B build -DLLAMA_BUILD_SHARED_LIB=on .
cmake --build build --config Release
```

Ensure `libllama.so` is present in `../llama.cpp/build/bin` before building this project.

```sh
make
```

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
