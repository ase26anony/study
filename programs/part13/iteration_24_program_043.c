This code appears to be part of a control flow graph (CFG) or fixup graph visualization/printing routine in a compiler or program analysis tool. Here's what each condition does:

## Purpose
The code prints special labels for specific nodes in a graph instead of their numeric indices.

## Special Node Types

1. **`ENTRY`** - The original entry block
2. **`ENTRY''`** - A modified/duplicate entry block (likely for analysis purposes)
3. **`EXIT`** - The original exit block  
4. **`EXIT''`** - A modified/duplicate exit block
5. **`NEW_EXIT`** - A newly created exit node in the fixup graph
6. **`NEW_ENTRY`** - A newly created entry node in the fixup graph

## Context Clues
- `fixup_graph` suggests this is part of a graph transformation/optimization pass
- The use of `2 * EXIT_BLOCK` and `+ 1` indicates the original blocks might be duplicated or paired with shadow nodes
- This is likely from GCC or LLVM code dealing with control flow graphs

## Typical Use Case
This pattern is common in:
- **Maximum flow algorithms** (like min-cut/max-flow for register allocation)
- **Control flow graph transformations**
- **Graph visualization/debugging utilities**

The code helps human readers understand the graph structure by labeling special nodes with meaningful names rather than numeric IDs.
