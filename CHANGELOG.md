# Changelog

The format is a section per release, newest first, written by hand. The release workflow reads the section matching the tag and uses it as the release notes, so a missing section fails the release rather than producing an empty one.

Nothing here is stable before 1.0. The crates are published so that the compiler can depend on them by version rather than by path, and their Rust APIs will change without a major version bump until the target list stops moving.

## Unreleased

`rucc-abi` is new, and it is the first half of milestone M6: the psABIs as data. An ABI is a description with register banks, how a scalar spends them, and two ordered lists of rules saying what an aggregate has to look like and how it travels if it does. One classifier reads any description. Five ABIs are described, which is the four the compiler implements by hand today plus Darwin arm64, and they add up to about two hundred lines of data.

The split is between mechanism and policy rather than between code and no code. The SysV eightbyte merge, the homogeneous aggregate scan, the RISC-V floating point pair and the exact size check are real algorithms and stay as code, and there are four of them across the five ABIs. What became data is which of those an ABI applies, in what order, with what limits, and what happens when the registers a rule wanted are not there. That last one is the part that is easiest to get wrong, because every test anybody writes by hand passes few enough arguments that it never comes up: SysV lets a later argument have the register an aggregate could not use, and AAPCS64 and RISC-V take the rest of the bank with them.

The evidence that the replacement is faithful is that it answers the same thing. The compiler's own test cases for its four hand written classifiers are ported across, thirty of them, including the ones that exist because somebody got them wrong once. There is also a test that adds a sixth ABI, s390x, entirely in the test file, and if that test ever needs a change under `src/` then bringing up an ABI has stopped being a data change.

`rucc-abi` also carries the type layouts, which is the half of a psABI that decides whether a target's own headers are read correctly rather than whether a call works. Sizes, alignments, whether a plain `char` is signed, whether a symbol gets a leading underscore, which end of a storage unit a bit-field starts at, and the cap on member alignment. A `long double` carries its width and its format separately, because sixteen bytes on x86-64 Linux, sixteen bytes on AArch64 Linux and sixteen bytes on 64-bit PowerPC are three different formats and a description carrying only the width would call them the same.

One bug fixed along the way. A plain `char` was reported unsigned on AArch64 Darwin, following AAPCS64, and Apple keeps it signed for source compatibility with the Intel Macs. So `aarch64-macos` and `aarch64-linux-gnu` are the same architecture with opposite answers, which is the pair most likely to catch a corpus out.

`rucc-cross abi <tuple>` prints all of it. The layout half prints for every target because it comes from the tuple, and the ABI half prints only where there is a description and says so plainly where there is not. Answering a question about an ABI with the almost-right answer is the one failure mode this crate exists to avoid.

## 0.1.0

The first release, and the whole of milestone M0: the target model.

`rucc-tuple` replaces the three field triple with a ten field tuple. The fields are the architecture, the baseline within it, the byte order, the data model, the operating system, the OS version, the environment, the environment version, the float ABI and the object format. The membership rule is the one in `spec/03-target-model.md`: a fact is in the tuple if it changes how a function is called or how a struct is laid out, and it is a flag otherwise. That rule is what makes a tuple usable as a cache key, which is what the sysroot cache and the stub generator both need.

Three things the old model got wrong are fixed by having the fields at all. The pointer width now comes from the data model rather than from the architecture, so a 64-bit Windows target correctly has 64-bit pointers and a 32-bit `long`, and `x86_64-linux-gnux32` correctly has 32-bit pointers on a 64-bit machine. The byte order is a field rather than a constant, so s390x is big-endian and `powerpc64le` and `aarch64_be` are spellable. The address width is exposed separately from the pointer width, which costs one function today and saves an audit of every use of the pointer width if CHERI ever comes into scope.

The parser no longer discards anything. The old one had a catch-all arm that skipped components it did not recognize, so `x86_64-linux-gnu.2.28` parsed as `x86_64-linux-gnu` and the pinned glibc version vanished with no diagnostic. Every component is now either understood or named in an error, and combinations that do not describe a machine, such as `x86_64-macos-musl` or a float ABI on an architecture with one calling convention, are refused with the component that was wrong.

It still accepts every spelling anyone else writes. A vendor is skipped, `x86_64-unknown-linux-gnu` and `x86_64-linux-gnu` are the same target, a Darwin kernel version is converted to the macOS product version it belongs to, and the mingw spellings imply their environment. There is one canonical output spelling and a separate LLVM spelling, because the tuple has to leave the building and the tools it goes to speak LLVM's dialect.

The target table is data. Forty two rows, each with the tier it is at today, the tier the plan commits to, whether rucc runs there, and why the row is interesting. `docs/TARGETS.md` is generated from it by `cargo xtask targets` and CI checks that it is up to date, because `spec/04-target-matrix.md` section 4.7 asks for a published table that cannot drift from the one the code reads.

`rucc-cross` is the command line tool, with `targets`, `info`, `canonical` and `llvm-triple`. `info` is the tier 4 promise: a target nothing is emitted for still answers correctly, so a user can find out what rucc thinks their machine is before finding out there is no back end for it.

The repository also arrives with its CI. A per-commit gate on five host configurations, a nightly that runs the tests on a big-endian host under emulation and builds the static musl artifact, a release workflow, and `cargo xtask style` enforcing the prose rules on every markdown file.
