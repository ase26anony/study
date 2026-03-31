This code appears to be from a compiler or program analysis tool that's printing special block names in a control flow graph. Here's what each condition does:

## Special Block Identifiers

1. **`ENTRY_BLOCK`** - The original entry block of the function
2. **`ENTRY_BLOCK + 1`** - A modified entry block (likely with some transformations)
3. **`2 * EXIT_BLOCK`** - The original exit block
4. **`2 * EXIT_BLOCK + 1`** - A modified exit block
5. **`fixup_graph->new_exit_index`** - A newly created exit block in a fixup graph
6. **`fixup_graph->new_entry_index`** - A newly created entry block in a fixup graph

## Context
This is likely from:
- **GCC (GNU Compiler Collection)** or similar compiler infrastructure
- A **fixup graph** used in optimization passes (like register allocation, instruction scheduling, or dataflow analysis)
- Part of a **graph dumping/debugging** function that prints human-readable names for special nodes

## Purpose
The code helps visualize special nodes when dumping graph representations for debugging. Instead of showing numeric indices for these important blocks, it shows descriptive names like "ENTRY", "EXIT", etc.

## Example Output
When `n` equals these special values, the output would be:
- `ENTRY` instead of some number
- `EXIT` instead of another number  
- `NEW_ENTRY` for newly created entry points
- etc.

This makes debug output much more readable when analyzing control flow graphs during compiler development.
