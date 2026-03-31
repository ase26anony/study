This code appears to be part of a control flow graph (CFG) or fixup graph visualization/printing routine in a compiler or program analysis tool. Here's what each condition does:

## Purpose
The code prints special labels for specific nodes in a graph instead of their numeric indices.

## Special Node Types

1. **`ENTRY`** - The original entry block
2. **`ENTRY''`** - A modified/duplicate entry block (likely for analysis purposes)
3. **`EXIT`** - The original exit block  
4. **`EXIT''`** - A modified/duplicate exit block
5. **`NEW_EXIT`** - A newly created exit node in the fixup graph
6. **`NEW_ENTRY`** - A newly created entry node in the fixup graph

## Why This Pattern?
This is common in compiler optimization passes where:
- Graphs are transformed for analysis (like adding artificial entry/exit nodes)
- Multiple versions of blocks exist during different optimization phases
- Special handling is needed for control flow edges

## Typical Context
This might be found in:
- GCC's control flow graph infrastructure
- LLVM's CFG manipulation passes  
- Minimum cut/maximum flow algorithms for register allocation
- Data flow analysis frameworks

The `fixup_graph` structure suggests this is part of a graph transformation where artificial nodes are added to simplify algorithms (like adding a super-source and super-sink for flow problems).
