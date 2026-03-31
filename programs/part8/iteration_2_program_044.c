This code appears to be from a compiler or program analysis tool that works with control flow graphs. It's handling special block indices when printing/displaying graph nodes. Here's what each condition does:

## Special Block Identifiers:

1. **`ENTRY_BLOCK`** - Prints "ENTRY" for the main entry block
2. **`ENTRY_BLOCK + 1`** - Prints "ENTRY''" (likely a duplicate or special entry variant)
3. **`2 * EXIT_BLOCK`** - Prints "EXIT" for the main exit block
4. **`2 * EXIT_BLOCK + 1`** - Prints "EXIT''" (likely a duplicate or special exit variant)
5. **`fixup_graph->new_exit_index`** - Prints "NEW_EXIT" for a newly created exit block
6. **`fixup_graph->new_entry_index`** - Prints "NEW_ENTRY" for a newly created entry block

## Context:
This is likely from GCC or LLVM code that:
- Handles control flow graph transformations
- Creates "fixup" graphs for optimization or analysis
- Needs to distinguish between original and transformed blocks
- The doubling (`2 * EXIT_BLOCK`) suggests some form of node splitting or duplication

## Purpose:
The code provides human-readable labels for special graph nodes when outputting graph information (for debugging, visualization, or logging purposes).
