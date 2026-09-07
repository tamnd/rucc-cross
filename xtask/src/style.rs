//! The prose rules.
//!
//! Three of them, all mechanical, all checked on every markdown file in the repository. They
//! are house style rather than good taste in general, and the reason they are enforced by a
//! program is that house style applied by hand is house style applied unevenly.
//!
//! The dash rule and the horizontal rule check are the same two the compiler repository's
//! `cargo xtask style` runs, deliberately, so that a paragraph can move between the two
//! repositories without being reformatted. The wrapping rule is new here.

use std::path::Path;
use std::process::ExitCode;

use crate::markdown_files;

/// One thing wrong with one line.
struct Finding {
    path: String,
    line: usize,
    message: &'static str,
}

/// Check every markdown file and report every finding, rather than stopping at the first.
///
/// Reporting all of them matters more than it sounds. A checker that stops at the first
/// problem turns a five minute fix into five round trips through CI, and the second time that
/// happens people stop running it locally.
pub fn run(root: &Path) -> ExitCode {
    let mut findings = Vec::new();

    for path in markdown_files(root) {
        let Ok(text) = std::fs::read_to_string(&path) else {
            continue;
        };
        let display = path.strip_prefix(root).unwrap_or(&path).display().to_string();
        check(&display, &text, &mut findings);
    }

    if findings.is_empty() {
        println!("style: clean");
        return ExitCode::SUCCESS;
    }

    for finding in &findings {
        println!("{}:{}: {}", finding.path, finding.line, finding.message);
    }
    println!();
    println!("{} problems", findings.len());
    ExitCode::FAILURE
}

/// Apply the three rules to one file.
fn check(path: &str, text: &str, findings: &mut Vec<Finding>) {
    let lines: Vec<&str> = text.lines().collect();
    let mut in_fence = false;

    for (index, line) in lines.iter().enumerate() {
        let number = index + 1;
        let trimmed = line.trim_start();

        if trimmed.starts_with("```") || trimmed.starts_with("~~~") {
            in_fence = !in_fence;
            continue;
        }
        if in_fence {
            continue;
        }

        // An em dash or an en dash. Every use of one is a comma, a colon, a full stop or a pair
        // of brackets that the writer did not choose between, and choosing is the writing.
        if line.contains('\u{2014}') {
            findings.push(Finding { path: path.into(), line: number, message: "em dash" });
        }
        if line.contains('\u{2013}') {
            findings.push(Finding { path: path.into(), line: number, message: "en dash" });
        }

        // A horizontal rule, which is a page break in a document nobody prints. The first line
        // is exempt because a `---` there opens a front matter block.
        if number > 1 && is_horizontal_rule(trimmed) {
            findings.push(Finding {
                path: path.into(),
                line: number,
                message: "horizontal rule, which is a page break",
            });
        }

        // A sentence continued onto the next line. Hard wrapped prose produces diffs where a
        // one word edit rewrites a paragraph, and it is unreadable in a pull request.
        if index + 1 < lines.len() && !line.trim().is_empty() {
            let next = lines[index + 1];
            if !next.trim().is_empty()
                && !is_structural(next)
                && !is_structural(line)
                && !next.trim_start().starts_with("```")
            {
                findings.push(Finding {
                    path: path.into(),
                    line: number + 1,
                    message: "a sentence continues onto a new line, so the paragraph is wrapped",
                });
            }
        }
    }
}

/// Whether a line is `---`, `***` or `___` on its own.
fn is_horizontal_rule(line: &str) -> bool {
    let trimmed = line.trim_end();
    if trimmed.len() < 3 {
        return false;
    }
    ["-", "*", "_"].iter().any(|c| trimmed.chars().all(|ch| ch.to_string() == **c))
}

/// Whether a line is markdown structure rather than a sentence.
///
/// Table rows, list items, headings, block quotes, indented code and link definitions all
/// legitimately sit next to each other on consecutive lines, and none of them is a wrapped
/// sentence.
fn is_structural(line: &str) -> bool {
    if line.starts_with("    ") || line.starts_with('\t') {
        return true;
    }
    let trimmed = line.trim_start();
    if trimmed.starts_with('|')
        || trimmed.starts_with('#')
        || trimmed.starts_with('>')
        || trimmed.starts_with("- ")
        || trimmed.starts_with("* ")
        || trimmed.starts_with("+ ")
        || trimmed.starts_with("<!--")
        || trimmed.starts_with('[')
        || trimmed.starts_with("```")
        || trimmed.starts_with("~~~")
    {
        return true;
    }
    // An ordered list item: digits, then a dot or a bracket, then a space.
    let digits: String = trimmed.chars().take_while(char::is_ascii_digit).collect();
    if !digits.is_empty() {
        let rest = &trimmed[digits.len()..];
        if rest.starts_with(". ") || rest.starts_with(") ") {
            return true;
        }
    }
    false
}
