This code appears to be from a compiler or program analysis tool that's working with control flow graphs. It's handling special block indices when printing/displaying graph nodes. Here's what each condition does:

## Special Block Indices Being Checked:

1. **`ENTRY_BLOCK`** - Prints "ENTRY" for the main entry block
2. **`ENTRY_BLOCK + 1`** - Prints "ENTRY''" (likely a secondary entry point)
3. **`2 * EXIT_BLOCK`** - Prints "EXIT" for the main exit block  
4. **`2 * EXIT_BLOCK + 1`** - Prints "EXIT''" (likely a secondary exit point)
5. **`fixup_graph->new_exit_index`** - Prints "NEW_EXIT" for a newly created exit
6. **`fixup_graph->new_entry_index`** - Prints "NEW_ENTRY" for a newly created entry

## Context:
This is likely from:
- **GCC (GNU Compiler Collection)** or similar compiler infrastructure
- A **fixup graph** used in optimization passes (like maximum flow/min-cut algorithms)
- Part of a **graph dumping/debugging** function that prints human-readable node names

## Purpose:
The code provides meaningful labels for special nodes in control flow graphs rather than just printing numeric indices, making debug output more readable for developers working on compiler optimizations.

The `fixup_graph` structure suggests this is part of a graph transformation or optimization pass that creates new entry/exit nodes during analysis.
