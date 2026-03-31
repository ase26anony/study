This code appears to be from a compiler or graph analysis tool (likely GCC or similar) that's handling control flow graph (CFG) nodes with special indices. Here's what each condition checks:

## Special Node Indices

1. **`ENTRY_BLOCK`** - The main entry block of the function
2. **`ENTRY_BLOCK + 1`** - A secondary entry block (labeled "ENTRY''")
3. **`2 * EXIT_BLOCK`** - The main exit block
4. **`2 * EXIT_BLOCK + 1`** - A secondary exit block (labeled "EXIT''")
5. **`fixup_graph->new_exit_index`** - A newly created exit node in a fixup graph
6. **`fixup_graph->new_entry_index`** - A newly created entry node in a fixup graph

## Context
This is likely from:
- **Fixup graph construction** for minimum cost flow problems
- **Control flow graph transformations** (like adding artificial source/sink nodes)
- **Graphviz/dot output generation** for debugging CFGs

## Purpose
The code prints human-readable labels for special nodes when outputting graph data (probably for visualization or debugging). Normal basic blocks would be printed with their numeric indices, but these special nodes get descriptive names.

## Typical Use Case
In compiler optimization passes that use network flow algorithms (like register allocation, instruction scheduling), artificial entry/exit nodes are often added to transform the problem into a standard min-cost flow formulation.
