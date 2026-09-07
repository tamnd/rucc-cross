# Contributing

## Getting set up

```
cargo build --workspace
cargo test --workspace
cargo xtask style
cargo xtask targets --check
```

That is the whole gate. If those four pass locally they pass in CI, and if they do not then either the gate is wrong or your machine is, and both are worth knowing about before you open a pull request.

The toolchain is pinned to Rust 1.98.0 in `rust-toolchain.toml`, so rustup will fetch it the first time you build. The floor is 1.85.0 and CI checks it separately.

## Where code goes

`crates/rucc-tuple` is the target model. It has no dependencies, it is what the compiler imports, and everything else in the repository reads from it. A change here that adds a target is a data change and should touch only the table.

`crates/rucc-cross` is the command line tool. It is mostly print queries today, and it grows a subcommand per milestone as the stub generator, the sysroot assembler and the differential harness arrive. Each of those lives in its own crate and the tool wires it up, so that the logic is testable without a process.

`xtask` is the house rules. Adding one is fine, and it should be a check somebody can run and not just a paragraph in a document.

`spec/` is the seventeen specification documents. They are the argument for what is being built and why, and changing what the code does without changing the document that promised something else leaves the next person reading a lie.

## The prose rules

`cargo xtask style` checks three things on every markdown file, and they are the same rules the compiler repository enforces, so that a paragraph can move between the two without being reformatted.

No em dash and no en dash. Every use of one is a comma, a colon, a full stop or a pair of brackets that the writer did not choose between, and choosing is the writing.

No horizontal rules. A `---` line is a page break in a document nobody prints.

No hard wrapping. One sentence continues to the end of the line, however long that is. Wrapped prose produces diffs where a one word edit rewrites a paragraph, which is unreadable in a pull request.

The same rules apply to pull request bodies, issue comments and commit messages, where no tool checks them. Write like a person explaining something to a colleague.

## Comments

The rule is that a comment says why, and the code already says what. A comment that restates the line under it is noise, and a comment explaining why a psABI requires something that looks wrong is the most valuable thing in the file.

Where a decision was close, say what the alternative was and why it lost. The next person to read it will have the same idea you had and will spend an afternoon on it otherwise.

## Pull requests

One reviewable change per pull request. If the body needs the word "also" more than once, it is two pull requests.

Every pull request names the milestone it belongs to, links the tracking issue, and gets the labels for its area and its milestone. When it merges, the tracking issue's checklist gets ticked and a comment goes on the issue saying what landed and what it means for the milestone. That is how the issue stays the truth about progress rather than a list somebody wrote once.

A pull request that changes the target table has to explain the evidence. `spec/04-target-matrix.md` section 4.7 is explicit that a tier is computed from corpus results and never asserted, so raising one in a table edit is the one change that will be sent back without discussion.

## Releases

The version is one number for the whole workspace. A patch release, `v0.x.y`, is cut when enough has landed to be worth a tag. A minor release, `v0.x.0`, is cut when a milestone in `spec/15-plan.md` is finished.

Neither is automatic. The changelog entry is where somebody writes down what the release means, and a release with a generated list of commit subjects instead of that is a release nobody can read.
