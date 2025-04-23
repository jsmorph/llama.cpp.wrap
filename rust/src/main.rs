use std::io::{self, BufRead};
use llama_wrapper::complete;

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

    let stdin = io::stdin();
    let mut lines = stdin.lock().lines();

    while let Some(line) = lines.next() {
        let line = line?;
        if line.is_empty() {
            continue;
        }

        match complete(&line, model_path, n_ctx) {
            Ok(resp_json) => println!("{}", resp_json),
            Err(e) => eprintln!("Error: {}", e),
        }
    }

    Ok(())
}
