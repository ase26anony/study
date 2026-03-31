This code appears to be part of a control flow graph (CFG) or fixup graph visualization/printing routine in a compiler or program analysis tool. Here's what each condition is checking:

## Graph Node Types

The code handles special nodes in a fixup/flow graph:

1. **`ENTRY_BLOCK`** - The original entry block of the function
2. **`ENTRY_BLOCK + 1`** - A modified/duplicate entry block (denoted with double quotes)
3. **`2 * EXIT_BLOCK`** - The original exit block
4. **`2 * EXIT_BLOCK + 1`** - A modified/duplicate exit block
5. **`fixup_graph->new_exit_index`** - A newly created exit node in the fixup graph
6. **`fixup_graph->new_entry_index`** - A newly created entry node in the fixup graph

## Context

This is likely from:
- **GCC (GNU Compiler Collection)** or similar compiler infrastructure
- A **minimum cost flow** or **fixup graph** implementation for register allocation or optimization
- Part of a graph dumping/debugging function that prints human-readable labels instead of numeric indices

## Purpose

The fixup graph is typically used in:
- **Register allocation** (e.g., for spilling code)
- **Control flow optimization**
- Creating **single-entry, single-exit** regions for analysis

The special nodes (`NEW_ENTRY`, `NEW_EXIT`) are often added to transform the original CFG into a form suitable for specific algorithms that require unique source/sink nodes.
