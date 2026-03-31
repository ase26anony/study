This code appears to be part of a control flow graph or fixup graph visualization/printing routine. It handles special cases for printing node labels based on their indices. Here's what each condition does:

## Special Node Indices

1. **`ENTRY_BLOCK`** - Prints "ENTRY" for the main entry block
2. **`ENTRY_BLOCK + 1`** - Prints "ENTRY''" for a secondary entry block
3. **`2 * EXIT_BLOCK`** - Prints "EXIT" for the main exit block  
4. **`2 * EXIT_BLOCK + 1`** - Prints "EXIT''" for a secondary exit block
5. **`fixup_graph->new_exit_index`** - Prints "NEW_EXIT" for a newly created exit node
6. **`fixup_graph->new_entry_index`** - Prints "NEW_ENTRY" for a newly created entry node

## Context
This is likely from a compiler optimization pass (like GCC) that:
- Performs control flow graph transformations
- Creates fixup graphs for algorithms like min-cut or flow analysis
- Needs to handle special nodes during graph dumping/debugging

## Typical Use Case
This pattern is common in:
- **Register allocation** (graph coloring with spill code)
- **Control flow optimization** (edge splitting, critical edge removal)
- **Data flow analysis** (adding artificial entry/exit nodes)

The code ensures special nodes get meaningful labels when dumping the graph for debugging or visualization purposes, rather than just printing numeric indices.
