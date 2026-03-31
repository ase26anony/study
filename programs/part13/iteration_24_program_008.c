This code appears to be part of a control flow graph (CFG) or fixup graph visualization/printing routine. It handles special block indices with descriptive labels. Here's what each case represents:

## Special Block Indices:

1. **`ENTRY_BLOCK`** → "ENTRY"
   - The original entry block of the function/CFG

2. **`ENTRY_BLOCK + 1`** → "ENTRY''"
   - Likely a duplicated or transformed version of the entry block

3. **`2 * EXIT_BLOCK`** → "EXIT"
   - The original exit block (scaled by 2, possibly due to edge splitting)

4. **`2 * EXIT_BLOCK + 1`** → "EXIT''"
   - A duplicated or transformed version of the exit block

5. **`fixup_graph->new_exit_index`** → "NEW_EXIT"
   - A newly created exit block in the fixup graph

6. **`fixup_graph->new_entry_index`** → "NEW_ENTRY"
   - A newly created entry block in the fixup graph

## Context:
This is likely from a **compiler optimization pass** (GCC or similar) that:
- Transforms control flow graphs
- Adds artificial entry/exit nodes for analysis
- Performs edge splitting or other CFG transformations
- The fixup graph is probably used for **minimum cost flow** or **register allocation** problems

## Why the scaling (2×)?
The `2 * EXIT_BLOCK` pattern suggests edge splitting where each original edge is represented as two nodes/edges in the transformed graph, possibly for:
- Capacity constraints in flow problems
- Handling of critical edges
- Creating a bipartite graph structure

This code helps debug/visualize the transformed graph by giving meaningful names to special nodes rather than just printing numeric indices.
