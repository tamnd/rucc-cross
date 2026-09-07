//! The command line tool.
//!
//! Today it answers questions about targets, which is all there is to answer while the target
//! model is the only thing built. As the milestones land it grows the commands that generate
//! stubs, assemble sysroots and run the differential harness, and each of those arrives with the
//! crate that implements it rather than as a pile of logic in this file.
//!
//! The argument parsing is by hand. There are four subcommands and no flags with values, so a
//! dependency here would be a dependency in the release artifact and in the audit surface, for
//! a `match` on a string. `Cargo.toml` explains the rule and the day it stops being the right
//! one is a day worth a paragraph in the pull request.

use std::process::ExitCode;
use std::str::FromStr;

use rucc_tuple::{TARGETS, TargetTuple, lookup, planned_working_count};

const USAGE: &str = "\
rucc-cross, the cross compilation tooling for rucc

usage:
  rucc-cross targets              list every target and what is true of it
  rucc-cross info <tuple>         everything the compiler derives from one tuple
  rucc-cross canonical <tuple>    the canonical spelling
  rucc-cross llvm-triple <tuple>  the LLVM spelling, for tools that want one
  rucc-cross --version
  rucc-cross --help
";

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let borrowed: Vec<&str> = args.iter().map(String::as_str).collect();

    match borrowed.as_slice() {
        [] | ["--help"] | ["-h"] | ["help"] => {
            print!("{USAGE}");
            ExitCode::SUCCESS
        }
        ["--version"] | ["-V"] => {
            println!("rucc-cross {}", env!("CARGO_PKG_VERSION"));
            ExitCode::SUCCESS
        }
        ["targets"] => {
            targets();
            ExitCode::SUCCESS
        }
        ["info", tuple] => run(tuple, info),
        ["canonical", tuple] => run(tuple, |t| println!("{}", t.to_canonical_string())),
        ["llvm-triple", tuple] => run(tuple, |t| println!("{}", t.to_llvm_string())),
        _ => {
            eprint!("{USAGE}");
            ExitCode::FAILURE
        }
    }
}

/// Parse a tuple and hand it to a command, or print the diagnostic and fail.
///
/// The diagnostic is the whole point of the exercise, so it goes to standard error unadorned
/// and the exit status is non zero. A tool that prints an error and exits zero is a tool that
/// breaks a build script silently.
fn run(tuple: &str, body: impl FnOnce(TargetTuple)) -> ExitCode {
    match TargetTuple::from_str(tuple) {
        Ok(target) => {
            body(target);
            ExitCode::SUCCESS
        }
        Err(error) => {
            eprintln!("error: {error}");
            eprintln!("  in target tuple `{tuple}`");
            ExitCode::FAILURE
        }
    }
}

/// Print the table, and the counts computed from it rather than written next to it.
fn targets() {
    let width = TARGETS.iter().map(|entry| entry.tuple.len()).max().unwrap_or(0);
    println!("{:width$}  now  plan  host   notes", "target");
    for entry in TARGETS {
        println!(
            "{:width$}  {:^3}  {:^4}  {:<5}  {}",
            entry.tuple,
            entry.tier.number(),
            entry.planned.number(),
            entry.host,
            entry.note
        );
    }
    println!();
    println!(
        "{} rows, {} of which the plan compiles and links",
        TARGETS.len(),
        planned_working_count()
    );
}

/// Print everything the compiler derives from one tuple.
///
/// This is the tier 4 promise from `spec/04-target-matrix.md`: a target that emits nothing still
/// answers this correctly, so a user can find out what rucc thinks their machine is before
/// finding out there is no back end for it.
fn info(target: TargetTuple) {
    let model = target.data_model();
    let row = lookup(&target);

    println!("canonical      {}", target.to_canonical_string());
    println!("llvm           {}", target.to_llvm_string());
    println!("arch           {}", target.arch());
    if target.sub_arch() != rucc_tuple::SubArch::None {
        println!("baseline       {}", target.sub_arch().as_str());
    }
    println!("endian         {}", target.endian());
    println!("os             {}", target.os());
    if let Some(version) = target.os_version() {
        println!("os version     {version}");
    }
    println!(
        "env            {}",
        if target.env().as_str().is_empty() { "none" } else { target.env().as_str() }
    );
    if let Some(version) = target.env_version() {
        println!("env version    {version}");
    }
    if target.arch().selects_float_abi() {
        println!("abi            {}", target.resolved_abi());
    }
    println!("object format  {}", target.object_format());
    println!("data model     {model}");
    println!("int            {} bits", model.int_width());
    println!("long           {} bits", model.long_width());
    println!("long long      {} bits", model.long_long_width());
    println!("pointer        {} bits", model.pointer_width());
    println!("address        {} bits", model.address_width());
    println!("char           {}", if target.char_is_signed() { "signed" } else { "unsigned" });
    println!("underscore     {}", if target.leading_underscore() { "yes" } else { "no" });

    match row {
        Some(entry) => {
            println!("tier           {} ({})", entry.tier, entry.tier.describe());
            println!("planned tier   {} ({})", entry.planned, entry.planned.describe());
            println!("host           {}", entry.host);
            if !entry.note.is_empty() {
                println!("note           {}", entry.note);
            }
        }
        None => {
            println!("tier           not in the table");
        }
    }
}
