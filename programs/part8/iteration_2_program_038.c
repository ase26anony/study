This code appears to be from a compiler or program analysis tool that's printing special block names in a control flow or fixup graph. Here's what each condition does:

## Special Block Identifiers

1. **`ENTRY_BLOCK`** - Prints "ENTRY" for the main entry block
2. **`ENTRY_BLOCK + 1`** - Prints "ENTRY''" (likely a duplicate or special entry variant)
3. **`2 * EXIT_BLOCK`** - Prints "EXIT" for the main exit block
4. **`2 * EXIT_BLOCK + 1`** - Prints "EXIT''" (likely a duplicate or special exit variant)
5. **`fixup_graph->new_exit_index`** - Prints "NEW_EXIT" for a newly created exit
6. **`fixup_graph->new_entry_index`** - Prints "NEW_ENTRY" for a newly created entry

## Context
This is likely from:
- **GCC** or **LLVM** compiler infrastructure
- A **fixup graph** used in optimization passes (like register allocation, instruction scheduling, or control flow transformations)
- Part of a **graph dumping/debugging** routine

## Purpose
The code handles special cases when printing node/block identifiers, giving meaningful names to:
- Original entry/exit blocks
- Their duplicates (common in graph transformations)
- Newly created entry/exit nodes during graph modifications

The `''` suffix suggests these might be "shadow" or "clone" blocks created during some transformation phase.
