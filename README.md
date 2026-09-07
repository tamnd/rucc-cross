# rucc-cross

Cross compilation for [rucc](https://github.com/tamnd/rucc): the target model, the ABI descriptions, the libc stubs, the sysroots and the harnesses that test all of it.

The goal is one sentence. `rucc --target=aarch64-linux-musl main.c -o main` on a Windows laptop with no toolchain installed, no sysroot flag and no second download, producing a static binary that runs on a Raspberry Pi. `zig cc` already does that and carries LLVM to do it. Nothing does it in twenty five megabytes.

## Why this is a separate repository

Three reasons, and the first one is the practical one.

Testing a cross compiler means QEMU, foreign sysroots, downloaded SDKs, emulated hosts and CI jobs that take an hour. None of that belongs in the gate that has to stay under twenty minutes on every commit to the compiler. The compiler repository keeps its per-commit CI; this repository is where the slow and wide matrix lives.

The second is that the code here is genuinely separable. The target tuple, the ABI descriptions and the stub generator are libraries with no dependency on the compiler's internals, so they can be built and tested on their own, and the compiler picks them up as ordinary crates when each one is ready.

The third is licensing. A compiler repository that vendors glibc headers is a compiler repository nobody can read the license of. The same argument that put the compatibility corpora in [rucc-compat](https://github.com/tamnd/rucc-compat) puts the sysroot machinery here.

## The specification

`spec/` carries seventeen documents. Start with `spec/02-the-goal.md` for the five claims and how each one is falsified, then `spec/04-target-matrix.md` for the target list and the entry price for each tier. `spec/15-plan.md` has the milestone map and the cost.

References of the form "parent document 12" point at `spec/` in the compiler repository. References of the form "document 06" point inside this one.

## The claims

Five, each decided by a number or by the exit status of a command.

1. For every target triple `zig cc` will compile and link a hosted C program for, rucc does the same, from every host zig runs on, out of one binary, with no external toolchain.
2. The code quality and compile throughput numbers of the compiler do not regress by more than two percent as a result of any change made for this work.
3. Bringing up target N takes fewer engineer days than target N minus one for every N of four or more, and the per target line count outside the target crate and the target's rule set is zero.
4. No target is listed as supported until rung 0 and rung 1 of the target ladder pass on it, and the report says whether that was on hardware or under emulation.
5. Two invocations for the same target and the same input on different hosts produce byte identical output, and every input that is not ours is named with its source, its hash and its licence.

Claim 2 is the one that can end the project. It is stated second rather than last for that reason, and `spec/02-the-goal.md` section 2.7 writes down what happens if it fails.

## Layout

```
crates/rucc-tuple    the target tuple, the target table and the tier model
crates/rucc-cross    the command line tool, which is mostly print queries for now
spec                 the seventeen documents
docs                 generated, and TARGETS.md is the target table as published
xtask                the house rules: prose style, and the checks CI runs
```

More crates arrive with the milestones they belong to. `spec/15-plan.md` says which.

`CONTRIBUTING.md` has the gate you can run locally, the prose rules and what a pull request is expected to carry.

## Building

```
cargo build --workspace
cargo test --workspace
cargo run -p rucc-cross -- targets
```

The toolchain is pinned to Rust 1.98.0 in `rust-toolchain.toml`. The floor that CI checks separately is 1.85.0, because a tool people cannot install on the machine where they need it is a tool they do not use.

## Status

M0 is the target model. Everything after it is in the milestone list, and each milestone has a tracking issue whose checklist is the truth about what is done.

Nothing here is stable. The crates are published so that the compiler can depend on them by version rather than by path, and their Rust APIs will change without a major version bump until the target list stops moving.

## Licence

Apache-2.0. See `LICENSE-APACHE`.
