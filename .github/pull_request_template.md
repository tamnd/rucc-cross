## What this changes

<!-- One paragraph. What is different after this merges, in terms a reader who has not seen the code will understand. -->

## Why

<!-- The reason, not the mechanism. If a decision was close, say what the alternative was and why it lost. -->

## Evidence

<!-- What you ran and what it said. A change to a facts file is evidence. A change to an assertion is evidence when the compiler output that forced it is quoted beside it. -->

## Milestone

<!-- Which milestone in spec/cross-compile/15-plan.md in the compiler repository, and the tracking issue this advances. -->

Part of tamnd/rucc#

## Checklist

- [ ] `bin/lint` passes
- [ ] `bin/facts --check` passes, and any diff it printed is explained above
- [ ] `bin/compile-corpus` passes
- [ ] `bin/run-corpus` passes, or says which rows it skipped and why
