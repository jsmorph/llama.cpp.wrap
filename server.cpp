#include "wrap.h"
#include "llama.h"
#include "httplib.h"
#include "json.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <optional>

// Use the nlohmann json library
using json = nlohmann::json;

// Logging macros
#define LOG_DEBUG(msg) do { std::cout << "[DEBUG] " << msg << std::endl; } while(0)
#define LOG_INFO(msg) do { std::cout << "[INFO] " << msg << std::endl; } while(0)
#define LOG_ERROR(msg) do { std::cerr << "[ERROR] " << msg << std::endl; } while(0)

// Helper function to escape JSON strings
std::string escape_json(const std::string& s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        if (*c == '"') {
            o << "\\\"";
        } else if (*c == '\\') {
            o << "\\\\";
        } else if (*c == '\b') {
            o << "\\b";
        } else if (*c == '\f') {
            o << "\\f";
        } else if (*c == '\n') {
            o << "\\n";
        } else if (*c == '\r') {
            o << "\\r";
        } else if (*c == '\t') {
            o << "\\t";
        } else if ('\x00' <= *c && *c <= '\x1f') {
            o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(*c);
        } else {
            o << *c;
        }
    }
    return o.str();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        LOG_ERROR("Usage: " << argv[0] << " <model_path> [port]");
        fprintf(stderr, "Usage: %s <model_path> [port]\n", argv[0]);
        return 1;
    }
    
    std::string model_path = argv[1];
    int port = argc >= 3 ? std::atoi(argv[2]) : 8080;
    
    LOG_INFO("Starting server with model: " << model_path << " on port: " << port);
    
    LOG_INFO("Initializing llama backend");
    llama_backend_init();
    
    LOG_INFO("Creating llama_state with model: " << model_path);
    llama_state* state = make_llama(model_path.c_str(), 2048);
    if (!state) {
        LOG_ERROR("Failed to initialize llama_state");
        fprintf(stderr, "Failed to initialize llama_state\n");
        llama_backend_free();
        return 1;
    }
    LOG_INFO("llama_state created successfully");
    

// Request structure for completion endpoint
struct CompletionRequest {
    std::string prompt;
    int max_tokens = 128;
    float temperature = 0.8f;
    float top_p = 0.9f;
    int top_k = 40;
    int seed = -1;
    bool include_logits = false;
    
    // Constructor with default values
    CompletionRequest() = default;
    
    // Parse from JSON
    static std::optional<CompletionRequest> from_json(const json& j) {
        CompletionRequest req;
        
        // Required field
        if (!j.contains("prompt") || !j["prompt"].is_string()) {
            return std::nullopt;
        }
        req.prompt = j["prompt"].get<std::string>();
        
        // Optional fields with defaults
        if (j.contains("max_tokens") && j["max_tokens"].is_number_integer()) {
            req.max_tokens = j["max_tokens"].get<int>();
        }
        
        if (j.contains("temperature") && j["temperature"].is_number_float()) {
            req.temperature = j["temperature"].get<float>();
        }
        
        if (j.contains("top_p") && j["top_p"].is_number_float()) {
            req.top_p = j["top_p"].get<float>();
        }
        
        if (j.contains("top_k") && j["top_k"].is_number_integer()) {
            req.top_k = j["top_k"].get<int>();
        }
        
        if (j.contains("seed") && j["seed"].is_number_integer()) {
            req.seed = j["seed"].get<int>();
        }
        
        if (j.contains("include_logits") && j["include_logits"].is_boolean()) {
            req.include_logits = j["include_logits"].get<bool>();
        }
        
        return req;
    }
};


    
    LOG_INFO("Setting up HTTP server");
    httplib::Server server;
    
    // API endpoint for completion - accepts JSON POST requests
    server.Post("/complete", [&](const httplib::Request& req, httplib::Response& res) {
        LOG_INFO("Received request to /complete endpoint");
        LOG_INFO("Request body: '" << req.body << "'");

        auto content_type = req.get_header_value("Content-Type");
        if (content_type.find("application/json") == std::string::npos) {
            LOG_ERROR("Invalid content type: " << content_type);
            res.status = 415;
            res.set_content("{\"error\":\"Content-Type must be application/json\"}", "application/json");
            return;
        }

        LOG_INFO("Parsing JSON request");

        try {
            json request_json = json::parse(req.body);
            auto request_opt = CompletionRequest::from_json(request_json);
            if (!request_opt) {
                LOG_ERROR("Invalid request format: missing or invalid 'prompt' field");
                res.status = 400;
                res.set_content("{\"error\":\"missing or invalid 'prompt' field\"}", "application/json");
                return;
            }
            const auto& request = *request_opt;

            LOG_INFO("Request parameters:");
            LOG_INFO("  prompt: '" << request.prompt << "'");
            LOG_INFO("  max_tokens: " << request.max_tokens);
            LOG_INFO("  temperature: " << request.temperature);
            LOG_INFO("  top_p: " << request.top_p);
            LOG_INFO("  top_k: " << request.top_k);
            LOG_INFO("  seed: " << request.seed);
            LOG_INFO("  include_logits: " << (request.include_logits ? "true" : "false"));

            LOG_INFO("Calling complete()");
            completion_params params;
            params.prompt = request.prompt.c_str();
            params.max_tokens = request.max_tokens;
            params.temperature = request.temperature;
            params.top_p = request.top_p;
            params.top_k = request.top_k;
            params.seed = request.seed;
            params.include_logits = request.include_logits;

            completion_result result;
            int rc = complete(state, &params, &result);
            if (rc != 0) {
                LOG_ERROR("complete() failed with code " << rc);
                res.status = 500;
                res.set_content("{\"error\":\"internal completion error\"}", "application/json");
                return;
            }

            json response_json;
            response_json["text"] = result.text ? std::string(result.text) : "";
            response_json["seed"] = request.seed;
            if (result.tokens_json) {
                response_json["tokens"] = json::parse(result.tokens_json);
            }

            if (result.text) free(result.text);
            if (result.tokens_json) free(result.tokens_json);

            res.set_content(response_json.dump(), "application/json");

        } catch (const json::parse_error& e) {
            LOG_ERROR("Failed to parse JSON: " << e.what());
            res.status = 400;
            res.set_content("{\"error\":\"invalid JSON format: " + std::string(e.what()) + "\"}", "application/json");
            return;
        } catch (const std::exception& e) {
            LOG_ERROR("Error processing request: " << e.what());
            res.status = 500;
            res.set_content("{\"error\":\"internal server error: " + std::string(e.what()) + "\"}", "application/json");
            return;
        }
    });
    
    // Add a simple health check endpoint
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        LOG_INFO("Health check request received");
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });
    
    LOG_INFO("Server running at http://localhost:" << port);
    printf("Server running at http://localhost:%d/\n", port);
    server.listen("0.0.0.0", port);
    
    LOG_INFO("Cleaning up resources");
    free_llama(state);
    llama_backend_free();

    return 0;
}
