package main

/*
#cgo LDFLAGS: -L${SRCDIR}/.. -lwrap -lllama -lstdc++
#include "../wrap.h"
#include <stdlib.h>
*/
import "C"
import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"unsafe"
)

const MaxLineLen = 65536

func main() {
	// Check command line arguments
	if len(os.Args) != 3 {
		fmt.Fprintf(os.Stderr, "Usage: %s <model_path> <context_size>\n", os.Args[0])
		os.Exit(1)
	}

	modelPath := os.Args[1]
	nCtx, err := strconv.Atoi(os.Args[2])
	if err != nil || nCtx <= 0 {
		fmt.Fprintf(os.Stderr, "Invalid context size: %s\n", os.Args[2])
		os.Exit(1)
	}

	// Initialize the model
	cModelPath := C.CString(modelPath)
	defer C.free(unsafe.Pointer(cModelPath))

	state := C.make_llama(cModelPath, C.int(nCtx))
	if state == nil {
		fmt.Fprintf(os.Stderr, "Failed to initialize model\n")
		os.Exit(1)
	}
	defer C.free_llama(state)

	// Process input lines
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := scanner.Text()

		// Parse completion parameters from JSON
		var params C.struct_completion_params
		cLine := C.CString(line)
		parseOk := C.parse_completion_params_json(cLine, &params)
		C.free(unsafe.Pointer(cLine))

		if parseOk != 0 {
			fmt.Fprintf(os.Stderr, "Failed to parse completion request JSON\n")
			continue
		}

		// Perform completion
		var result C.struct_completion_result
		compOk := C.complete(state, &params, &result)
		C.free(unsafe.Pointer(params.prompt))

		if compOk != 0 {
			fmt.Fprintf(os.Stderr, "Completion failed\n")
			continue
		}

		// Serialize and output the result
		respJSON := C.serialize_completion_result_json(&result)
		if respJSON != nil {
			fmt.Println(C.GoString(respJSON))
			C.free(unsafe.Pointer(respJSON))
		} else {
			fmt.Fprintf(os.Stderr, "Failed to serialize completion result\n")
		}

		// Free result resources
		if result.text != nil {
			C.free(unsafe.Pointer(result.text))
		}
		if result.tokens_json != nil {
			C.free(unsafe.Pointer(result.tokens_json))
		}
	}

	if err := scanner.Err(); err != nil {
		fmt.Fprintf(os.Stderr, "Error reading input: %v\n", err)
		os.Exit(1)
	}
}
