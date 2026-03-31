This code appears to be part of a control flow graph or fixup graph visualization/printing routine in a compiler or program analysis tool. Here's what each condition does:

## Purpose
The code prints special labels for specific nodes in a graph instead of their numeric indices.

## Special Node Identifiers

1. **`ENTRY_BLOCK`** → Prints "ENTRY"
   - The main entry point of the function/CFG

2. **`ENTRY_BLOCK + 1`** → Prints "ENTRY''"
   - Likely a duplicate or secondary entry point

3. **`2 * EXIT_BLOCK`** → Prints "EXIT"
   - The main exit point

4. **`2 * EXIT_BLOCK + 1`** → Prints "EXIT''"
   - Likely a duplicate or secondary exit point

5. **`fixup_graph->new_exit_index`** → Prints "NEW_EXIT"
   - A newly created exit node in the fixup graph

6. **`fixup_graph->new_entry_index`** → Prints "NEW_ENTRY"
   - A newly created entry node in the fixup graph

## Context
This is likely from:
- **GCC** or **LLVM** compiler infrastructure
- A **minimum cost flow** or **fixup graph** implementation
- Graph algorithms for **register allocation** or **code motion**
- **Control flow graph** transformations

The "fixup_graph" suggests this is part of a graph used to fix/correct flow after transformations, possibly for:
- Inserting compensation code
- Handling critical edges
- Implementing code sinking or hoisting
- Preparing for register allocation

The pattern of having both regular and "prime" versions (`''`) suggests this might be implementing a **node splitting** or **edge splitting** transformation common in compiler optimizations.
