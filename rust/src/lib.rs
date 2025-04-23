use libc::{c_char, c_float, c_int, c_void};
use std::ffi::{CStr, CString};
use std::ptr;

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
    #[link_name = "complete"]
    pub fn complete_c(
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

#[derive(Debug)]
pub enum LlamaError {
    InvalidContextSize,
    ModelInitFailed,
    ParseJsonFailed,
    CompletionFailed,
    SerializeFailed,
    Utf8Error,
    Other(String),
}

impl std::fmt::Display for LlamaError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self)
    }
}

impl std::error::Error for LlamaError {}

pub struct LlamaModel {
    state: *mut LlamaState,
}

impl LlamaModel {
    pub fn new(model_path: &str, n_ctx: i32) -> Result<Self, LlamaError> {
        if n_ctx <= 0 {
            return Err(LlamaError::InvalidContextSize);
        }
        let model_path_c = CString::new(model_path).map_err(|e| LlamaError::Other(e.to_string()))?;
        let state = unsafe { make_llama(model_path_c.as_ptr(), n_ctx) };
        if state.is_null() {
            Err(LlamaError::ModelInitFailed)
        } else {
            Ok(LlamaModel { state })
        }
    }

    pub fn complete(&self, request_json: &str) -> Result<String, LlamaError> {
        let json_str = CString::new(request_json).map_err(|e| LlamaError::Other(e.to_string()))?;
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
            return Err(LlamaError::ParseJsonFailed);
        }

        let mut result = CompletionResult {
            text: ptr::null_mut(),
            tokens_json: ptr::null_mut(),
        };

        let comp_result = unsafe { complete_c(self.state, &params, &mut result) };

        unsafe {
            if !params.prompt.is_null() {
                libc::free(params.prompt as *mut c_void);
            }
        }

        if comp_result != 0 {
            return Err(LlamaError::CompletionFailed);
        }

        let resp_json = unsafe { serialize_completion_result_json(&result) };
        let output = if !resp_json.is_null() {
            let c_str = unsafe { CStr::from_ptr(resp_json) };
            let str_slice = c_str.to_str().map_err(|_| LlamaError::Utf8Error)?;
            let s = str_slice.to_owned();
            unsafe {
                libc::free(resp_json as *mut c_void);
            }
            Ok(s)
        } else {
            Err(LlamaError::SerializeFailed)
        };

        unsafe {
            if !result.text.is_null() {
                libc::free(result.text as *mut c_void);
            }
            if !result.tokens_json.is_null() {
                libc::free(result.tokens_json as *mut c_void);
            }
        }

        output
    }
}

impl Drop for LlamaModel {
    fn drop(&mut self) {
        unsafe {
            free_llama(self.state);
        }
    }
}

/// Complete a request given a model path, context size, and request JSON.
/// Returns the response JSON or an error.
pub fn complete(request_json: &str, model_path: &str, n_ctx: i32) -> Result<String, LlamaError> {
    let model = LlamaModel::new(model_path, n_ctx)?;
    model.complete(request_json)
}
