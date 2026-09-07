# Contributing

## The gate

```
bin/lint
toolchains/install zig
bin/facts --check
bin/compile-corpus
bin/run-corpus
```

That is the whole of it, and CI runs the same five commands. The last one needs qemu and prints `skipped` for every row it has no emulator for, which is every row on a mac.

If `bin/facts --check` fails, read the diff before you run `bin/facts --record`. A facts file changing means either the reference compiler moved or a target's definition did, and both are worth a sentence in the commit message. Recording a diff you did not read turns a finding into a fact.

## Adding a toolchain

```
toolchains/record-hash gcc 16.1 x86_64-linux https://example.invalid/gcc-16.1.tar.xz
```

Paste the line it prints into `toolchains/manifest` and say in the commit message where the URL came from and who publishes it. The script does not edit the manifest on purpose: a file a script appends to is a file nobody reviews, and the point of pinning is that somebody looked.

## Adding a target

Add the row to `targets` with the spelling for every tool that has one. Do not write a spelling you have not run. The zig column was built by trying each candidate and keeping the ones that worked, and three of them did not, which is a fact worth having rather than a gap to paper over.

Then run `bin/facts --record` and `bin/compile-corpus` and commit what comes out.

## Adding to the corpus

An assertion needs a reason next to it, in a sentence, in the file. `_Static_assert(_Alignof(double) == 4)` on its own is a number somebody will delete the next time it fails. The same line with the sentence about the 1990 System V i386 psABI beside it is a claim somebody has to argue with.

An executing test has to produce the same output on every target. If it cannot, it is testing a layout rather than a calling convention, and it belongs in `corpus/layout` where the divergence is written down as a condition rather than in an expected file where it is written down as a number.

## Writing

Plain English, the way a developer writes to another developer. No em dashes and no en dashes: a sentence that wants one wanted a comma, a colon, a period or the word "to". No horizontal rules, because a heading says what the next part is and a rule says only that there is one. No line break in the middle of a sentence, so a paragraph is one line however long it gets. `bin/lint` checks the first two and the third is on you.

This is the same set of rules the compiler repository checks in `cargo xtask style`, so a paragraph can move between the two without being reformatted on arrival.

## Shell

POSIX sh, checked with `shellcheck -s sh`. No bashisms, because one of the hosts is a mac with an old bash and another is git-bash on Windows.

If you find yourself wanting a real language, stop and ask whether the thing you are writing belongs in `tamnd/rucc` instead. That is where the crates are and that is where a program with a build system should go.

## Pull requests

One thing per pull request, with the evidence in the body. A change to a facts file is evidence. A change to an assertion is evidence when the compiler output that forced it is quoted beside it.
