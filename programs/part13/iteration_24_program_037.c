This code appears to be part of a control flow graph or fixup graph visualization/printing routine in a compiler or program analysis tool. Here's what each condition does:

## Purpose
The code prints special labels for specific nodes in a graph rather than their numeric indices.

## Special Node Identifiers

1. **`ENTRY`** - The main entry block
2. **`ENTRY''`** - A secondary entry block (likely entry+1)
3. **`EXIT`** - The main exit block
4. **`EXIT''`** - A secondary exit block (likely exit+1)
5. **`NEW_EXIT`** - A newly created exit node in the fixup graph
6. **`NEW_ENTRY`** - A newly created entry node in the fixup graph

## Context Clues
- `fixup_graph` suggests this is part of a graph transformation/optimization pass
- The use of `2 * EXIT_BLOCK` suggests EXIT_BLOCK might be a special constant
- `new_exit_index` and `new_entry_index` are dynamically created nodes
- This is likely from GCC or a similar compiler infrastructure

## Typical Use Case
This pattern is common in:
- Control flow graph manipulation
- Maximum flow/minimum cut algorithms (like in register allocation)
- Graph rewriting passes
- Debug output for compiler developers

The code helps developers understand graph transformations by showing meaningful names instead of raw indices when dumping graph information to a file.
