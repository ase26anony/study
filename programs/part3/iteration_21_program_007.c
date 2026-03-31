This code appears to be part of a graph visualization or debugging output routine, likely from a compiler or optimization pass. It's handling special node indices in a fixup graph (probably related to control flow graph transformations).

Here's what each condition does:

1. **`n == ENTRY_BLOCK`** → Prints "ENTRY" for the main entry block
2. **`n == ENTRY_BLOCK + 1`** → Prints "ENTRY''" for a secondary entry block (possibly a cloned or duplicated entry)
3. **`n == 2 * EXIT_BLOCK`** → Prints "EXIT" for the main exit block  
4. **`n == 2 * EXIT_BLOCK + 1`** → Prints "EXIT''" for a secondary exit block
5. **`n == fixup_graph->new_exit_index`** → Prints "NEW_EXIT" for a newly created exit node
6. **`n == fixup_graph->new_entry_index`** → Prints "NEW_ENTRY" for a newly created entry node

This looks like it's from **GCC's (GNU Compiler Collection) fixup graph implementation**, likely in the context of:
- **Control flow graph (CFG) transformations**
- **Register allocation or instruction scheduling**
- **Graph algorithms** (like max-flow/min-cut for register allocation)

The pattern suggests the graph has been transformed with:
- Original blocks duplicated (ENTRY'' and EXIT'')
- New artificial entry/exit nodes added for algorithm purposes
- Special indices used to distinguish between different types of nodes

This is common in compiler optimization passes that use graph algorithms, where artificial source/sink nodes are added to simplify the problem formulation.
