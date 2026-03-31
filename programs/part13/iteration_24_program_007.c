This code appears to be part of a graph visualization or debugging output routine, likely from a compiler optimization pass (possibly related to control flow or fixup graphs). Here's what each condition does:

## Purpose
The code prints special labels for specific nodes in a graph instead of their numeric indices, making the output more readable.

## Special Node Identifiers

1. **`ENTRY_BLOCK`** → Prints "ENTRY"
   - The main entry point of the graph

2. **`ENTRY_BLOCK + 1`** → Prints "ENTRY''"
   - Likely a secondary entry point or modified entry

3. **`2 * EXIT_BLOCK`** → Prints "EXIT"
   - The main exit point (note the multiplication by 2)

4. **`2 * EXIT_BLOCK + 1`** → Prints "EXIT''"
   - Likely a secondary exit point

5. **`fixup_graph->new_exit_index`** → Prints "NEW_EXIT"
   - A newly created exit node in the fixup graph

6. **`fixup_graph->new_entry_index`** → Prints "NEW_ENTRY"
   - A newly created entry node in the fixup graph

## Context
This is likely from GCC or LLVM code dealing with:
- **Fixup graphs**: Used in optimization passes like maximum flow algorithms
- **Control flow graphs (CFG)**: Representing program flow with basic blocks
- **Graph transformations**: Where new entry/exit nodes are created during optimization

The multiplication by 2 for exit blocks suggests the graph might use a split representation where each original node is represented by two nodes (in/out) for flow algorithms.
