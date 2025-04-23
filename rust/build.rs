fn main() {
    // Build the wrap static library
    println!("cargo:rustc-link-search=.");
    println!("cargo:rustc-link-lib=static=wrap");
    
    // Link with llama.cpp libraries
    println!("cargo:rustc-link-search=../../llama.cpp/build-static/src");
    println!("cargo:rustc-link-search=../../llama.cpp/build-static/ggml/src");
    println!("cargo:rustc-link-lib=static=llama");
    println!("cargo:rustc-link-lib=static=ggml");
    println!("cargo:rustc-link-lib=static=ggml-base");
    println!("cargo:rustc-link-lib=static=ggml-cpu");
    
    // Link with system libraries
    println!("cargo:rustc-link-lib=pthread");
    println!("cargo:rustc-link-lib=gomp");  // OpenMP library
    println!("cargo:rustc-flags=-l dylib=stdc++");
    
    // Rerun build script if wrap.h or wrap.cpp changes
    println!("cargo:rerun-if-changed=../wrap.h");
    println!("cargo:rerun-if-changed=../wrap.cpp");
}
