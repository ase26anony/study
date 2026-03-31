This code appears to be part of a control flow graph (CFG) or fixup graph visualization/printing routine in a compiler or program analysis tool. Here's what each condition does:

## Purpose
The code prints special labels for specific nodes in a graph instead of their numeric indices, making the output more readable.

## Special Node Identifiers

1. **`ENTRY_BLOCK`** → Prints "ENTRY"
   - The main entry block of the function/CFG

2. **`ENTRY_BLOCK + 1`** → Prints "ENTRY''"
   - Likely a duplicate or special variant of the entry block

3. **`2 * EXIT_BLOCK`** → Prints "EXIT"
   - The main exit block

4. **`2 * EXIT_BLOCK + 1`** → Prints "EXIT''"
   - Likely a duplicate or special variant of the exit block

5. **`fixup_graph->new_exit_index`** → Prints "NEW_EXIT"
   - A newly created exit node in a fixup/transformed graph

6. **`fixup_graph->new_entry_index`** → Prints "NEW_ENTRY"
   - A newly created entry node in a fixup/transformed graph

## Context
This is likely from:
- **GCC (GNU Compiler Collection)** or similar compiler infrastructure
- Part of a **control flow graph manipulation** pass
- Used in **graph algorithms** like maximum flow/minimum cut
- The `fixup_graph` suggests this is for **fixing/transforming** a CFG for analysis

## Typical Use Case
When printing graph edges or nodes for debugging, these special labels help identify:
- Original vs. transformed graph nodes
- Artificial nodes added for algorithms
- Entry/exit points in various graph representations

The pattern suggests the graph uses a numbering scheme where special nodes have reserved indices, and transformed graphs add new artificial nodes with their own indices.
