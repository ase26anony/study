This code appears to be part of a control flow graph or fixup graph visualization/printing routine. It's handling special node indices with specific labels. Here's what each condition does:

## Special Node Indices and Their Labels:

1. **`ENTRY_BLOCK`** → Labeled as `"ENTRY"`
2. **`ENTRY_BLOCK + 1`** → Labeled as `"ENTRY''"` (double prime)
3. **`2 * EXIT_BLOCK`** → Labeled as `"EXIT"`
4. **`2 * EXIT_BLOCK + 1`** → Labeled as `"EXIT''"` (double prime)
5. **`fixup_graph->new_exit_index`** → Labeled as `"NEW_EXIT"`
6. **`fixup_graph->new_entry_index`** → Labeled as `"NEW_ENTRY"`

## Context:
This is likely from a **compiler optimization pass** (possibly GCC or LLVM) that:
- Works with control flow graphs
- Performs some transformation that requires "fixup" of edges/nodes
- Creates new entry/exit nodes during optimization
- Uses special indices for artificial/transformed nodes

## Common Use Cases:
1. **Minimum Cut/Maximum Flow algorithms** in register allocation or code motion
2. **Control flow graph transformations** that add artificial source/sink nodes
3. **Graph algorithms** that require special entry/exit handling
4. **Profile-guided optimization** with edge splitting

The double prime notation (`''`) suggests these might be **split nodes** or **duplicate nodes** created during some transformation (like edge splitting for better optimization).
