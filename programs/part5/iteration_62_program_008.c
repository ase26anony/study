This code appears to be part of a switch statement that counts different types in some kind of type analysis or statistics collection. Here's what it does:

## Summary
The code increments different counters based on the type being processed. Each `TYPE_*` case increments a corresponding counter variable.

## Breakdown

### Counter Variables
- `nb_undefined` - counts undefined types
- `nb_scalar` - counts scalar types (integers, floats, etc.)
- `nb_string` - counts string types
- `nb_struct` - counts struct types
- `nb_user_struct` - counts user-defined struct types
- `nb_union` - counts union types
- `nb_pointer` - counts pointer types
- `nb_array` - counts array types
- `nb_callback` - counts callback/function types
- `nb_lang_struct` - counts language-specific struct types

### Special Case
- `TYPE_NONE`: Uses `gcc_unreachable()` which is a GCC built-in that tells the compiler this code path should never be reached

## Possible Context
This looks like it could be from:
- A compiler or language tool (GCC-related based on `gcc_unreachable()`)
- A type system analyzer
- Code that collects statistics about types in a codebase
- Part of a serialization or marshaling system

## Missing Parts
The complete code would likely include:
- A switch statement declaration
- A variable being switched on (probably a type enum)
- Initialization of all counter variables
- Possibly a default case
- Code that uses these statistics after collection
