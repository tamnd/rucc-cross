# Changelog

The format is a section per change, newest first, written by hand. There are no releases and no version numbers, because there is nothing here to install. What a user of this repository consumes is the checkout itself, so the dates are the interesting axis and a tag would only be a second name for a commit.

## Unreleased

`bin/sysroot` produces a musl sysroot for a target and writes the manifest that says what went into it. `sysroots/manifest` pins the musl source by version, licence and sha256, the same shape `toolchains/manifest` uses, and the version is 1.2.5 because that is what Zig 0.16 carries and comparing against a different one would compare two things at once. The build uses the pinned zig as the cross compiler rather than whatever `cc` the machine has, for the same reason.

The layout it produces is the one `rucc-sysroot` in the compiler repository reads: `include/<arch>` for the headers that differ by architecture, `include/generic` for the copy every architecture shares, `lib` for the start files and the archives, and `manifest` at the top. musl installs one flat include tree with the architecture specific files in `bits/`, so the split is a move of that one directory.

This is the exit criterion of the sysroot half of tamnd/rucc#619, and running it found the thing it was built to find. All four musl targets were produced on a macOS arm64 laptop and on a Linux x86-64 box, and the first comparison had 219 identical headers and six different compiled artifacts. The difference was `DW_AT_comp_dir`: clang writes the working directory into every object, including the ones assembled from `.s` files, and the two machines have different home directories. With `-fdebug-compilation-dir=.` and `-ffile-prefix-map` the four targets reproduce byte for byte, manifest and contents, and `bin/sysroot --check` is the comparison as a command.

The repository stops being a Rust workspace and becomes what its name says: the toolchains, the environment and the C corpus that the compiler is compared against.

`crates/`, `xtask/`, `spec/`, `docs/`, `Cargo.toml`, `Cargo.lock` and the release workflow are gone. `rucc-tuple` and the target table moved to the compiler repository, where they belong, because they are compiler code and splitting the compiler across two repositories bought nothing except a version number between a crate and its only caller. The specification moved with them to `spec/cross-compile/`. Anything published in the 0.1.0 tag of this repository is superseded by that move, and the tag stays where it is as a record of where the code was.

What replaces it has no build system and no dependencies. `toolchains/manifest` pins the reference compilers by version and sha256, `toolchains/install` fetches and verifies them, and `targets` is the table of forty two targets with the spelling each reference wants for each one. Every zig spelling in that table was verified by running the compiler rather than read from documentation, which is how four spellings that do not exist and one that is not the obvious guess were found.

The corpus is in three parts. `corpus/layout/` is `_Static_assert` over sizes, alignments and signedness, with the reason for each number written beside it, and it compiles for every target without needing a machine to run on. `corpus/abi/` and `corpus/exec/` are programs that print values rather than sizes, so one expected transcript serves every target and a divergence is a miscounted register rather than a layout difference. `facts/` is the preprocessor's answer for each target, recorded, so a reference toolchain that changes its mind shows up as a diff instead of as a mystery.

Four drivers run all of it: `bin/facts`, `bin/compile-corpus`, `bin/run-corpus` and `bin/lint`. They are POSIX sh and checked with shellcheck, on the reasoning that a repository whose job is to be the thing you trust when the compiler under test is suspect should not have a toolchain of its own to be suspicious of.

The corpus found four bugs in its own assertions before it found anything else, which is the part worth recording. mingw aligns `double` and `long long` to eight bytes rather than following the System V i386 psABI. s390x caps scalar alignment at eight, so a sixteen byte `long double` and `__int128` are both eight aligned. wasm32 and the x32 ABI both have `__int128` with four byte pointers, so an assertion that tied the two together was simply wrong. It then found two in the compiler: s390x `char` is unsigned and `rucc-tuple` said signed, and `long double` on mingw is x87 extended while `rucc-abi` keyed the rule on the operating system alone and gave every Windows target the MSVC answer.

## 0.1.0

The first release, and the whole of milestone M0: the target model. Superseded by the move described above, and kept here because the tag exists.

`rucc-tuple` replaced the three field triple with a ten field tuple. The fields are the architecture, the baseline within it, the byte order, the data model, the operating system, the OS version, the environment, the environment version, the float ABI and the object format. The membership rule is that a fact is in the tuple if it changes how a function is called or how a struct is laid out, and it is a flag otherwise. That rule is what makes a tuple usable as a cache key, which is what the sysroot cache and the stub generator both need.

Three things the old model got wrong were fixed by having the fields at all. The pointer width came from the data model rather than from the architecture, so a 64-bit Windows target correctly had 64-bit pointers and a 32-bit `long`, and `x86_64-linux-gnux32` correctly had 32-bit pointers on a 64-bit machine. The byte order was a field rather than a constant, so s390x was big-endian and `powerpc64le` and `aarch64_be` were spellable. The address width was exposed separately from the pointer width.

The parser discarded nothing. The old one had a catch-all arm that skipped components it did not recognize, so `x86_64-linux-gnu.2.28` parsed as `x86_64-linux-gnu` and the pinned glibc version vanished with no diagnostic. Every component was either understood or named in an error, and combinations that do not describe a machine, such as `x86_64-macos-musl`, were refused with the component that was wrong.
