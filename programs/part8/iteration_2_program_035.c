This code appears to be part of a control flow graph or fixup graph implementation, likely from a compiler optimization pass. It's handling special block indices when printing/displaying graph nodes. Here's what each condition does:

## Special Block Identifiers:

1. **`ENTRY_BLOCK`** → Prints "ENTRY"
   - The main entry point of the function/CFG

2. **`ENTRY_BLOCK + 1`** → Prints "ENTRY''"
   - Possibly a duplicated or transformed entry block

3. **`2 * EXIT_BLOCK`** → Prints "EXIT"
   - The main exit point (multiplied by 2 suggests some indexing scheme)

4. **`2 * EXIT_BLOCK + 1`** → Prints "EXIT''"
   - Possibly a duplicated or transformed exit block

5. **`fixup_graph->new_exit_index`** → Prints "NEW_EXIT"
   - A newly created exit node in the fixup graph

6. **`fixup_graph->new_entry_index`** → Prints "NEW_ENTRY"
   - A newly created entry node in the fixup graph

## Context:
This is likely from **GCC (GNU Compiler Collection)** or similar compiler infrastructure, specifically from:
- A **minimum cost flow** or **maximum flow** algorithm implementation
- Part of **register allocation** or **instruction scheduling**
- The fixup graph is used to handle irregular control flow during optimization

## Purpose:
The code provides human-readable labels for special nodes when dumping/debugging the graph structure, making it easier to identify:
- Original vs. transformed blocks
- Special artificial nodes added for algorithmic purposes
- Entry/exit points in various representations

The use of `2 * EXIT_BLOCK` suggests the graph might be using a bipartite representation or edge-split transformation common in network flow algorithms applied to compiler graphs.
