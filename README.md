# rucc-cross

The toolchains, the environment and the C corpus that [rucc](https://github.com/tamnd/rucc) is compared against when it cross compiles. Zig 0.16, gcc, qemu, and the C that tells them apart.

There is no rucc code here. It moved. `rucc-tuple`, `rucc-abi`, the seventeen specification documents and the milestone issues all live in `tamnd/rucc`, in `crates/` and in `spec/cross-compile/`, because a crate the compiler depends on has no business being in a second repository and a version skew between the two is a bug waiting for a bad week. What is left here is the part that genuinely does not belong in a compiler checkout: a gigabyte of downloaded toolchains, an emulator per architecture, and the programs you run under them.

## What it is for

A cross compiler is a claim about a machine you are not standing next to. The only way to check one is to have something else compile the same source for the same machine and see whether the two agree, and then to run both and see whether they agree about that too. This repository is the something else.

Three things, and they are separable.

**The toolchains.** `toolchains/manifest` pins what we compare against, with a hash. `toolchains/install zig` fetches it and refuses anything whose hash does not match. Nothing is fetched from a URL that is not on a line somebody reviewed.

**The environment.** `targets` says how each of rucc's forty two targets is spelled to zig, to gcc, and to qemu. Every entry in the zig column was checked by running the compiler rather than read out of a manual, and the dashes are the ones that failed.

**The corpus.** `corpus/layout` is C that any correct compiler for a target must accept, written as assertions with the reason beside them. `corpus/abi` is C that has to produce the same output on every target, so a target that produces different output has miscounted a register rather than laid something out differently. `corpus/exec` is the smallest thing that proves a sysroot and a link line work at all.

**The sysroots.** `sysroots/manifest` pins the libc source with a hash, the same way the toolchains are pinned, and `bin/sysroot` builds one for a target and writes the record of what went into it. This is the producer that `rucc-sysroot` in the compiler repository deliberately does not contain, because fetching needs a cache, a provenance record and a network policy, and a crate the compiler links against should have none of those.

## Using it

```
toolchains/install zig          fetch the pinned reference compiler
bin/facts --all                 what the reference says about every target's scalar types
bin/facts --check               fail if anything in facts/ has drifted
bin/compile-corpus              compile the layout corpus for all forty two targets
bin/compile-abi-corpus          compile the compiler's generated layout corpus, one file per target
bin/run-corpus                  build and run the executing corpus under qemu
bin/sysroot aarch64-linux-musl  produce a musl sysroot, with a manifest
bin/sysroot --check a b         compare two manifests, which is the reproducibility check
bin/lint                        the house rules
```

`bin/compile-abi-corpus` is the one that reads the other repository. It takes the directory `cargo xtask abi-corpus` writes, which defaults to `../rucc/tests/abi-corpus` and can be given as an argument or in `RUCC_ABI_CORPUS`, and compiles each file for the target it is named after. The difference from `bin/compile-corpus` is what the C is: that one is C somebody wrote here, this one is C the compiler generated, and every line of it is a `_Static_assert` about a size, an alignment or a member offset that came out of `rucc_types::layout_record`. So a failure is not a corpus that will not build, it is the compiler and the reference disagreeing about a record, and the assertion that failed names it.

Set `RUCC_CROSS_REFERENCE=gcc` to compare against the platform's cross gcc instead of zig, for the rows where one exists. That column is much sparser than the zig one and the sparseness is the argument for this whole line of work: a per target gcc has to exist as a package before you can compare against it, and for most of the table nobody has built one.

The download cache is `$RUCC_CROSS_CACHE`, which defaults to `~/.cache/rucc-cross`. It is outside the checkout on purpose, so switching branches does not throw away a gigabyte.

## What it has found so far

The corpus earns its keep by being wrong in public. Six things, each of which was a plausible belief before the reference rejected it.

**Plain `char` is unsigned on s390x.** It was written down as signed, in with x86. The s390x ELF ABI says unsigned, and clang agrees, and the target had never been compiled for.

**mingw-w64 has an eighty bit `long double` and MSVC does not.** The rule was keyed on the operating system, so `x86_64-windows-gnu` got the eight byte answer that only `x86_64-windows-msvc` deserves. Two targets, one OS, different types.

**Windows on i386 aligns a `double` to eight and System V aligns it to four.** `__i386__` is not the condition. `__i386__ && !_WIN32` is. A struct of an int and a double is twelve bytes under one and sixteen under the other.

**s390x caps scalar alignment at eight.** An IEEE quad `long double` is sixteen bytes and aligned to eight there, and `__int128` is the same. It is the one target in the table where the size and the alignment of a scalar come apart in that direction.

**`__int128` is not a 64-bit only type.** wasm32 has it and so does `x86_64-linux-gnux32`, both with four byte pointers, which is a useful reminder that the pointer width and the widest integer are separate facts.

**A sysroot built on a mac and a sysroot built on a Linux box were different files with identical instructions.** The 219 headers matched on the first try and the six compiled artifacts did not, because clang writes the working directory into `DW_AT_comp_dir` of every object it produces, including the ones assembled from `.s` files, and `/Users/apple/...` is not `/home/tam/...`. `bin/sysroot` passes `-fdebug-compilation-dir=.` and `-ffile-prefix-map` for that reason, and with them all four musl targets reproduce byte for byte across the two hosts.

The first two were bugs in `rucc-tuple` and `rucc-abi` and are fixed. The next three were bugs in this corpus, and the comments in `corpus/layout/scalars.c` say so where they happened. The last one is why `bin/sysroot` has two flags in it that look like noise and are not.

## Layout

```
targets              how each rucc target is spelled to each tool, and what runs a binary for it
toolchains/manifest  what we compare against, pinned with a hash
toolchains/install   fetch and verify
bin/                 the drivers, all POSIX sh
corpus/layout        C that has to compile, with the reason for each assertion beside it
corpus/abi           C that has to produce the same output everywhere
corpus/exec          the smallest thing that proves a sysroot works
facts/               what the reference says about each target, recorded and checked in CI
```

Everything is POSIX sh. There is no build system and no dependency file, and that is deliberate: the day this repository needs a language with a build system is the day somebody should ask whether the thing being built belongs in `tamnd/rucc` instead.

## Where the design is written down

`spec/cross-compile/` in the compiler repository, starting at [`00-README.md`](https://github.com/tamnd/rucc/blob/main/spec/cross-compile/00-README.md). Section 14 is the testing document and it is the one this repository implements. The tracking issue is [tamnd/rucc#618](https://github.com/tamnd/rucc/issues/618).

## Licence

Apache-2.0. See `LICENSE-APACHE`. The corpus is ours. Nothing from a toolchain is vendored here, only fetched, and `toolchains/manifest` names the source of every byte.
