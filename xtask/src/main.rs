//! The house rules, run by CI and by hand.
//!
//! Two commands. `style` checks the prose, and `targets` regenerates the target table that the
//! documentation shows, so that the table in `docs/TARGETS.md` cannot drift from the one the
//! compiler reads. `spec/04-target-matrix.md` section 4.7 asks for exactly that: the table in
//! the specification is the plan, and every other copy of it is generated.

use std::path::{Path, PathBuf};
use std::process::ExitCode;

mod style;
mod targets;

const USAGE: &str = "\
usage:
  cargo xtask style             check the prose rules on every markdown file
  cargo xtask targets           regenerate docs/TARGETS.md from the target table
  cargo xtask targets --check   fail if docs/TARGETS.md is out of date
";

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let borrowed: Vec<&str> = args.iter().map(String::as_str).collect();
    let root = repo_root();

    match borrowed.as_slice() {
        ["style"] => style::run(&root),
        ["targets"] => targets::run(&root, false),
        ["targets", "--check"] => targets::run(&root, true),
        _ => {
            eprint!("{USAGE}");
            ExitCode::FAILURE
        }
    }
}

/// The workspace root, which is the parent of the directory holding this crate.
fn repo_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).parent().expect("xtask is not at the root").to_path_buf()
}

/// Every markdown file in the repository, in a stable order, skipping build output.
fn markdown_files(root: &Path) -> Vec<PathBuf> {
    let mut found = Vec::new();
    collect(root, &mut found);
    found.sort();
    found
}

fn collect(dir: &Path, found: &mut Vec<PathBuf>) {
    let Ok(entries) = std::fs::read_dir(dir) else {
        return;
    };
    for entry in entries.flatten() {
        let path = entry.path();
        let name = entry.file_name();
        let name = name.to_string_lossy();
        if name.starts_with('.') || name == "target" {
            continue;
        }
        if path.is_dir() {
            collect(&path, found);
        } else if path.extension().is_some_and(|ext| ext == "md") {
            found.push(path);
        }
    }
}
