#include "wrap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 65536

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <model_path> <context_size>\n", argv[0]);
        return 1;
    }
    const char* model_path = argv[1];
    int n_ctx = atoi(argv[2]);
    if (n_ctx <= 0) {
        fprintf(stderr, "Invalid context size: %s\n", argv[2]);
        return 1;
    }

    struct llama_state* state = make_llama(model_path, n_ctx);
    if (!state) {
        fprintf(stderr, "Failed to initialize model\n");
        return 1;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), stdin)) {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';

        struct completion_params params;
        memset(&params, 0, sizeof(params));
        int parse_ok = parse_completion_params_json(line, &params);
        if (parse_ok != 0) {
            fprintf(stderr, "Failed to parse completion request JSON\n");
            continue;
        }

        struct completion_result result;
        memset(&result, 0, sizeof(result));
        int comp_ok = complete(state, &params, &result);
        free((void*)params.prompt);

        if (comp_ok != 0) {
            fprintf(stderr, "Completion failed\n");
            continue;
        }

        char* resp_json = serialize_completion_result_json(&result);
        if (resp_json) {
            printf("%s\n", resp_json);
            free(resp_json);
        } else {
            fprintf(stderr, "Failed to serialize completion result\n");
        }

        if (result.text) free(result.text);
        if (result.tokens_json) free(result.tokens_json);
    }

    free_llama(state);
    return 0;
}
