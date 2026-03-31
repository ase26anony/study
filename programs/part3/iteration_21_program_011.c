This code appears to be part of a graph visualization or debugging output routine, likely from a compiler or optimization tool. Let me break down what it's doing:

## Purpose
The code is printing special node names for specific node indices in a graph structure, rather than just printing the raw index numbers.

## Special Node Identifiers

1. **`ENTRY`** - The main entry block of the function/CFG
2. **`ENTRY''`** - A secondary entry block (likely a duplicate or transformed version)
3. **`EXIT`** - The main exit block
4. **`EXIT''`** - A secondary exit block
5. **`NEW_EXIT`** - A newly created exit node in a fixup/transformed graph
6. **`NEW_ENTRY`** - A newly created entry node in a fixup/transformed graph

## Context Clues

- **`fixup_graph`** suggests this is part of a graph transformation/optimization pass
- The use of `2 * EXIT_BLOCK` and `2 * EXIT_BLOCK + 1` indicates the original graph might have been duplicated or transformed
- This is likely from GCC or LLVM compiler infrastructure
- The code handles special control flow graph (CFG) nodes that have been added or modified during optimization

## Likely Source
This looks like it could be from:
- GCC's `gcov` or profiling instrumentation
- A graph transformation pass (like minimum cut or flow analysis)
- Control flow graph manipulation for optimization

The pattern suggests the original graph has been transformed, possibly duplicated, with new entry/exit nodes added for analysis purposes.
