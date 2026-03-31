This code appears to be part of a control flow graph (CFG) or fixup graph visualization/printing routine in a compiler or program analysis tool. Here's what each condition is checking:

## Graph Node Types

1. **`ENTRY_BLOCK`** - The original entry block of the function
2. **`ENTRY_BLOCK + 1`** - A modified/transformed entry block (denoted with double quotes)
3. **`2 * EXIT_BLOCK`** - The original exit block
4. **`2 * EXIT_BLOCK + 1`** - A modified/transformed exit block
5. **`fixup_graph->new_exit_index`** - A newly created exit node in the fixup graph
6. **`fixup_graph->new_entry_index`** - A newly created entry node in the fixup graph

## Context
This is likely from:
- **GCC (GNU Compiler Collection)** - Specifically from the control flow graph or fixup graph handling
- **Graph transformation code** - Possibly for minimum cut/maximum flow algorithms used in register allocation or other optimizations
- **Debug/print functionality** - For visualizing the graph structure during development or debugging

## Purpose
The code prints human-readable labels for special nodes in a graph instead of just numeric indices. This helps when:
- Debugging graph transformations
- Understanding the structure of transformed control flow graphs
- Visualizing the results of graph algorithms

The double quotes (`''`) likely indicate duplicated or transformed versions of the original entry/exit blocks, which are common in graph algorithms that need to handle edge cases for source/sink nodes.
