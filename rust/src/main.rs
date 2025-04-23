use libc::{c_char, c_float, c_int, c_void};
use std::ffi::{CStr, CString};
use std::io::{self, BufRead};
use std::ptr;

// FFI bindings to the C structs and functions from wrap.h
#[repr(C)]
pub struct LlamaState {
    _private: [u8; 0],
}

#[repr(C)]
pub struct CompletionParams {
    pub prompt: *const c_char,
    pub max_tokens: c_int,
    pub temperature: c_float,
    pub top_p: c_float,
    pub top_k: c_int,
    pub seed: c_int,
    pub include_logits: bool,
}

#[repr(C)]
pub struct CompletionResult {
    pub text: *mut c_char,
    pub tokens_json: *mut c_char,
}

extern "C" {
    pub fn make_llama(model_path: *const c_char, n_ctx: c_int) -> *mut LlamaState;
    pub fn complete(
        state: *mut LlamaState,
        params: *const CompletionParams,
        result_out: *mut CompletionResult,
    ) -> c_int;
    pub fn free_llama(state: *mut LlamaState);
    pub fn parse_completion_params_json(
        json_str: *const c_char,
        out_params: *mut CompletionParams,
    ) -> c_int;
    pub fn serialize_completion_result_json(
        result: *const CompletionResult,
    ) -> *mut c_char;
}

// We're using the C functions directly for JSON parsing and serialization,
// so we don't need Rust structs for this purpose

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<String> = std::env::args().collect();
    if args.len() != 3 {
        eprintln!("Usage: {} <model_path> <context_size>", args[0]);
        return Err("Invalid arguments".into());
    }

    let model_path = &args[1];
    let n_ctx = args[2].parse::<i32>().map_err(|_| {
        eprintln!("Invalid context size: {}", args[2]);
        "Invalid context size"
    })?;

    if n_ctx <= 0 {
        eprintln!("Invalid context size: {}", args[2]);
        return Err("Invalid context size".into());
    }

    // Initialize the model
    let model_path_c = CString::new(model_path.as_str())?;
    let state = unsafe { make_llama(model_path_c.as_ptr(), n_ctx) };
    if state.is_null() {
        eprintln!("Failed to initialize model");
        return Err("Failed to initialize model".into());
    }

    // Process input lines
    let stdin = io::stdin();
    let mut lines = stdin.lock().lines();
    
    while let Some(line) = lines.next() {
        let line = line?;
        if line.is_empty() {
            continue;
        }

        // Parse the JSON request
        let json_str = CString::new(line.as_str())?;
        let mut params = CompletionParams {
            prompt: ptr::null(),
            max_tokens: 0,
            temperature: 0.0,
            top_p: 0.0,
            top_k: 0,
            seed: 0,
            include_logits: false,
        };

        let parse_result = unsafe {
            parse_completion_params_json(json_str.as_ptr(), &mut params)
        };

        if parse_result != 0 {
            eprintln!("Failed to parse completion request JSON");
            continue;
        }

        // Run the completion
        let mut result = CompletionResult {
            text: ptr::null_mut(),
            tokens_json: ptr::null_mut(),
        };

        let comp_result = unsafe { complete(state, &params, &mut result) };
        
        // Free the prompt string allocated by parse_completion_params_json
        unsafe {
            if !params.prompt.is_null() {
                libc::free(params.prompt as *mut c_void);
            }
        }

        if comp_result != 0 {
            eprintln!("Completion failed");
            continue;
        }

        // Serialize the result to JSON
        let resp_json = unsafe { serialize_completion_result_json(&result) };
        if !resp_json.is_null() {
            unsafe {
                let c_str = CStr::from_ptr(resp_json);
                if let Ok(str_slice) = c_str.to_str() {
                    println!("{}", str_slice);
                } else {
                    eprintln!("Failed to convert result to string");
                }
                libc::free(resp_json as *mut c_void);
            }
        } else {
            eprintln!("Failed to serialize completion result");
        }

        // Free the result strings
        unsafe {
            if !result.text.is_null() {
                libc::free(result.text as *mut c_void);
            }
            if !result.tokens_json.is_null() {
                libc::free(result.tokens_json as *mut c_void);
            }
        }
    }

    // Clean up
    unsafe {
        free_llama(state);
    }

    Ok(())
}
