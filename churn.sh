#!/bin/bash

# Get the same completion several times, and complain about any differences.

set -e

N=${1:-3}

mkdir -p scratch
for ((i=0; i<N; i++)); do
    curl -s -X POST "http://localhost:8080/complete" \
	 -H "Content-Type: application/json" \
	 -d '{"prompt":"Tell me a short story", "max_tokens":50, "temperature":0.7, "top_p":0.8, "top_k":30, "seed":1234, "include_logits":true}' \
	| tee scratch/test-$((i)).json | jq -c '.tokens[0:5]|map(.logit)'
    
    if (( i > 0 )); then
        prev="scratch/test-$((i-1)).json"
	this="scratch/test-$((i)).json"
        if ! diff -q "$prev" "$this" >/dev/null; then
            echo "Mismatch between $prev and $this"
            exit 1
        fi
    fi
done
