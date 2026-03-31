This code appears to be part of a control flow graph (CFG) or fixup graph visualization/printing routine in a compiler or program analysis tool. Here's what each condition is checking:

## Graph Node Types

The code handles special nodes in a graph structure:

1. **`ENTRY_BLOCK`** - The original entry block of the function/CFG
2. **`ENTRY_BLOCK + 1`** - A modified/duplicate entry block (denoted with double primes)
3. **`2 * EXIT_BLOCK`** - The original exit block
4. **`2 * EXIT_BLOCK + 1`** - A modified/duplicate exit block
5. **`fixup_graph->new_exit_index`** - A newly created exit node in a fixup graph
6. **`fixup_graph->new_entry_index`** - A newly created entry node in a fixup graph

## Context

This is likely from:
- **GCC (GNU Compiler Collection)** - Specifically from the control flow graph or fixup graph handling code
- **A fixup graph** - Used in algorithms like minimum cost flow for register allocation or other optimizations
- **Graph printing/debugging** - Outputting graph nodes with human-readable labels

## Purpose

The code prints descriptive labels for special nodes instead of numeric indices when outputting graph information to a file. This makes debug output more readable by showing:
- `"ENTRY"` instead of a number for the entry block
- `"EXIT"` instead of a number for the exit block  
- Special names for transformed/added nodes in fixup graphs

## Typical Use Case

This might be used in:
- Debug output for graph algorithms
- DOT file generation for graph visualization
- Logging during compiler optimization passes
- Debugging minimum cost flow or other graph transformation algorithms
