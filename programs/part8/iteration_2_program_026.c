This code appears to be part of a control flow graph or fixup graph implementation, likely from a compiler optimization pass. It's handling special block indices by printing descriptive names instead of numeric values. Here's what each condition does:

## Special Block Identifiers:

1. **`ENTRY_BLOCK`** - Prints "ENTRY" for the main entry block
2. **`ENTRY_BLOCK + 1`** - Prints "ENTRY''" (likely a duplicate or special entry variant)
3. **`2 * EXIT_BLOCK`** - Prints "EXIT" for the main exit block
4. **`2 * EXIT_BLOCK + 1`** - Prints "EXIT''" (likely a duplicate or special exit variant)
5. **`fixup_graph->new_exit_index`** - Prints "NEW_EXIT" for a newly created exit block
6. **`fixup_graph->new_entry_index`** - Prints "NEW_ENTRY" for a newly created entry block

## Context:
This is likely from GCC or a similar compiler infrastructure where:
- `ENTRY_BLOCK` and `EXIT_BLOCK` are predefined constants (probably 0 and 1)
- The fixup graph is used in optimization passes like maximum flow/minimum cut algorithms
- The code is part of debugging output to make graph dumps more readable

## Purpose:
The code helps visualize special nodes in graph dumps by replacing numeric indices with meaningful names, making it easier to debug control flow transformations and optimization passes.
