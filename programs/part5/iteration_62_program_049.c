This is a switch statement that increments different counters based on a type enumeration. Here's what it does:

**Purpose:** Counts occurrences of different data types in some kind of analysis or processing.

**Breakdown:**
- Each `case` handles a specific type from what appears to be a `TYPE_*` enumeration
- For each type, it increments a corresponding counter variable
- The `TYPE_NONE` case uses `gcc_unreachable()` which is a GCC compiler hint that this code path should never be reached

**Counter variables track:**
- `nb_undefined`: Undefined types
- `nb_scalar`: Scalar types (integers, floats, etc.)
- `nb_string`: String types
- `nb_struct`: Structure types
- `nb_user_struct`: User-defined structure types
- `nb_union`: Union types
- `nb_pointer`: Pointer types
- `nb_array`: Array types
- `nb_callback`: Callback/function pointer types
- `nb_lang_struct`: Language-specific structure types

**Missing context:**
- The switch variable isn't shown (likely something like `current_type`)
- The surrounding function and its purpose
- Where these counters are used after being incremented

This appears to be from a compiler, type analyzer, or code generator that needs to collect statistics about different types encountered during processing.
