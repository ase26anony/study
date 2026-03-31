This code appears to be part of a control flow graph or fixup graph visualization/printing routine. It's handling special node indices by printing descriptive labels instead of raw numbers. Here's what each case represents:

## Special Node Identifiers:

1. **`ENTRY_BLOCK`** → Prints "ENTRY"
   - The original entry block of the graph

2. **`ENTRY_BLOCK + 1`** → Prints "ENTRY''"
   - Likely a duplicated or transformed version of the entry block

3. **`2 * EXIT_BLOCK`** → Prints "EXIT"
   - The original exit block (multiplied by 2, possibly due to node splitting)

4. **`2 * EXIT_BLOCK + 1`** → Prints "EXIT''"
   - Likely a duplicated or transformed version of the exit block

5. **`fixup_graph->new_exit_index`** → Prints "NEW_EXIT"
   - A newly created exit node in the fixup graph

6. **`fixup_graph->new_entry_index`** → Prints "NEW_ENTRY"
   - A newly created entry node in the fixup graph

## Context:
This is likely from a compiler optimization pass (like GCC) that:
- Transforms control flow graphs
- Creates fixup graphs for optimizations like register allocation or instruction scheduling
- Uses node splitting or duplication techniques
- Needs to distinguish between original and transformed nodes when outputting debug information

The pattern suggests the graph uses a numbering scheme where:
- Original nodes might be at even indices
- Duplicated/transformed versions are at odd indices (n+1)
- Special artificial nodes (NEW_ENTRY/NEW_EXIT) are added for the fixup process

This code helps make debug output more readable by showing meaningful names instead of just numeric indices.
