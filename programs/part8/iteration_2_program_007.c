This code appears to be from a compiler or graph processing codebase (likely GCC or similar) that handles control flow graphs. It's printing special node names for specific indices in a fixup graph. Here's what each condition does:

## Special Node Identifiers

1. **`ENTRY_BLOCK`** - Prints "ENTRY" for the main entry block
2. **`ENTRY_BLOCK + 1`** - Prints "ENTRY''" (likely a duplicate or special entry variant)
3. **`2 * EXIT_BLOCK`** - Prints "EXIT" for the main exit block
4. **`2 * EXIT_BLOCK + 1`** - Prints "EXIT''" (likely a duplicate or special exit variant)
5. **`fixup_graph->new_exit_index`** - Prints "NEW_EXIT" for a newly created exit node
6. **`fixup_graph->new_entry_index`** - Prints "NEW_ENTRY" for a newly created entry node

## Context
This is likely part of:
- **Control flow graph (CFG) manipulation** in a compiler
- **Fixup graph** used in optimization passes (like maximum flow algorithms for register allocation or code motion)
- **Graph printing/debugging** utility to visualize special nodes

## Purpose
The code helps distinguish between:
- Original graph nodes vs. augmented nodes (NEW_ENTRY/NEW_EXIT)
- Regular blocks vs. special boundary blocks
- Possibly nodes created for graph transformations or optimization algorithms

This pattern is common in compiler infrastructure where graphs are augmented with additional nodes for algorithmic purposes, and debugging output needs to clearly identify these special nodes.
