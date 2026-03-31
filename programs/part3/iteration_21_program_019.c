This code appears to be from a compiler or program analysis tool that's printing special block names in a control flow or fixup graph. Here's what each condition does:

## Special Block Identifiers

1. **`ENTRY_BLOCK`** - Prints "ENTRY" for the main entry block
2. **`ENTRY_BLOCK + 1`** - Prints "ENTRY''" for a secondary entry block
3. **`2 * EXIT_BLOCK`** - Prints "EXIT" for the main exit block  
4. **`2 * EXIT_BLOCK + 1`** - Prints "EXIT''" for a secondary exit block
5. **`fixup_graph->new_exit_index`** - Prints "NEW_EXIT" for a newly created exit
6. **`fixup_graph->new_entry_index`** - Prints "NEW_ENTRY" for a newly created entry

## Context
This is likely from GCC or LLVM code that:
- Handles control flow graphs (CFGs)
- Performs transformations that add new entry/exit blocks
- Uses fixup graphs for flow analysis or optimization
- Has special handling for transformed/artificial blocks

## Purpose
The code ensures special blocks get meaningful names when printing/debugging graph structures, rather than just numeric indices. This helps developers understand the graph structure during debugging or when examining compiler dumps.
