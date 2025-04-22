#ifndef WRAP_H
#define WRAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

// Opaque struct for llama state
struct llama_state;

// Parameters for completion
struct completion_params {
    const char* prompt;
    int max_tokens;
    float temperature;
    float top_p;
    int top_k;
    int seed;
    bool include_logits;
};

// Result of completion
struct completion_result {
    char* text;         // malloc'd, must be freed by caller
    char* tokens_json;  // malloc'd, must be freed by caller (may be NULL if not requested)
};

// Create a llama_state (returns NULL on failure)
struct llama_state* make_llama(const char* model_path, int n_ctx);

// Run completion (returns 0 on success, nonzero on error)
int complete(struct llama_state* state, const struct completion_params* params, struct completion_result* result_out);

// Free a llama_state
void free_llama(struct llama_state* state);

int parse_completion_params_json(const char* json_str, struct completion_params* out_params);
char* serialize_completion_result_json(const struct completion_result* result);

#ifdef __cplusplus
}
#endif

#endif // WRAP_H
