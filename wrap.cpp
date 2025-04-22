#include "wrap.h"
#include "llama.h"
#include "json.hpp"
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

using json = nlohmann::json;

struct llama_state {
    llama_model* model;
    llama_context* ctx;
    const llama_vocab* vocab;
    int n_ctx;
};

struct TokenInfo {
    llama_token id;
    std::string text;
    float logit;
    json to_json() const {
        return {{"id", id}, {"text", text}, {"logit", logit}};
    }
};

extern "C" {

struct llama_state* make_llama(const char* model_path, int n_ctx) {
    llama_model_params model_params = llama_model_default_params();
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = n_ctx > 0 ? n_ctx : 2048;

    llama_model* model = llama_model_load_from_file(model_path, model_params);
    if (!model) return nullptr;

    llama_context* ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) {
        llama_model_free(model);
        return nullptr;
    }

    const llama_vocab* vocab = llama_model_get_vocab(model);
    if (!vocab) {
        llama_free(ctx);
        llama_model_free(model);
        return nullptr;
    }

    llama_state* state = new llama_state;
    state->model = model;
    state->ctx = ctx;
    state->vocab = vocab;
    state->n_ctx = ctx_params.n_ctx;
    return state;
}

int complete(struct llama_state* state, const struct completion_params* params, struct completion_result* result_out) {
    if (!state || !params || !result_out || !params->prompt) return 1;

    // Clear KV cache
    llama_kv_self_clear(state->ctx);

    // Format prompt
    std::string formatted_prompt = "Below is an instruction. Provide a response that appropriately completes the request.\n\n### Instruction:\n";
    formatted_prompt += params->prompt;
    formatted_prompt += "\n\n### Response:";

    // Tokenize prompt
    const int n_prompt = -llama_tokenize(state->vocab, formatted_prompt.c_str(), formatted_prompt.length(), nullptr, 0, true, true);
    if (n_prompt <= 0) return 2;

    std::vector<llama_token> prompt_tokens(n_prompt);
    int token_count = llama_tokenize(state->vocab, formatted_prompt.c_str(), formatted_prompt.length(), prompt_tokens.data(), prompt_tokens.size(), true, true);
    if (token_count < 0) return 3;

    llama_batch batch = llama_batch_get_one(prompt_tokens.data(), token_count);
    if (llama_decode(state->ctx, batch)) return 4;

    auto sparams = llama_sampler_chain_default_params();
    llama_sampler* sampler = llama_sampler_chain_init(sparams);
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(params->temperature));
    llama_sampler_chain_add(sampler, llama_sampler_init_top_p(params->top_p, 1));
    llama_sampler_chain_add(sampler, llama_sampler_init_top_k(params->top_k));
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(params->seed >= 0 ? (uint32_t)params->seed : 0));

    std::string output;
    std::vector<TokenInfo> tokens_info;

    for (int i = 0; i < params->max_tokens; ++i) {
        llama_token new_token = llama_sampler_sample(sampler, state->ctx, -1);
        if (llama_vocab_is_eog(state->vocab, new_token)) break;

        char token_text[128];
        int token_len = llama_token_to_piece(state->vocab, new_token, token_text, sizeof(token_text), 0, true);
        if (token_len < 0) token_len = 0;
        std::string piece(token_text, token_len);

        float token_logit = 0.0f;
        if (params->include_logits) {
            const float* logits = llama_get_logits_ith(state->ctx, -1);
            if (logits) {
                token_logit = logits[new_token];
                tokens_info.push_back(TokenInfo{new_token, piece, token_logit});
            }
        }

        output += piece;

        llama_batch next_batch = llama_batch_get_one(&new_token, 1);
        if (llama_decode(state->ctx, next_batch)) break;
    }

    llama_sampler_free(sampler);

    // Allocate and copy output text
    result_out->text = (char*)malloc(output.size() + 1);
    if (!result_out->text) return 5;
    std::memcpy(result_out->text, output.c_str(), output.size() + 1);

    // Allocate and copy tokens JSON if requested
    if (params->include_logits && !tokens_info.empty()) {
        json tokens_json = json::array();
        for (const auto& t : tokens_info) tokens_json.push_back(t.to_json());
        std::string tokens_str = tokens_json.dump();
        result_out->tokens_json = (char*)malloc(tokens_str.size() + 1);
        if (!result_out->tokens_json) {
            free(result_out->text);
            return 6;
        }
        std::memcpy(result_out->tokens_json, tokens_str.c_str(), tokens_str.size() + 1);
    } else {
        result_out->tokens_json = nullptr;
    }

    return 0;
}

void free_llama(struct llama_state* state) {
    if (!state) return;
    if (state->ctx) llama_free(state->ctx);
    if (state->model) llama_model_free(state->model);
    delete state;
}

} // extern "C"
