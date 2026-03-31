This code appears to be part of a control flow graph or fixup graph implementation, likely from a compiler optimization pass. It's handling special block indices when printing or outputting graph information. Here's what each condition does:

## Special Block Indices Being Checked:

1. **`ENTRY_BLOCK`** - Prints "ENTRY" for the main entry block
2. **`ENTRY_BLOCK + 1`** - Prints "ENTRY''" (likely a duplicate or special entry variant)
3. **`2 * EXIT_BLOCK`** - Prints "EXIT" for the main exit block
4. **`2 * EXIT_BLOCK + 1`** - Prints "EXIT''" (likely a duplicate or special exit variant)
5. **`fixup_graph->new_exit_index`** - Prints "NEW_EXIT" for a newly created exit block
6. **`fixup_graph->new_entry_index`** - Prints "NEW_ENTRY" for a newly created entry block

## Context:
This is likely from GCC or LLVM code that:
- Implements a **minimum cost flow** or **maximum flow** algorithm
- Creates a **fixup graph** for register allocation or other optimizations
- Uses special indices to represent artificial entry/exit nodes in the flow network

## Typical Use Case:
In compiler flow analysis, artificial entry/exit nodes are often added to:
- Handle multiple entry/exit points
- Transform control flow graphs into single-source/single-sink flow networks
- Implement algorithms like minimum cut or register spilling

The double notation (`''`) suggests these might be "shadow" or "mirror" nodes used in certain graph transformations.
