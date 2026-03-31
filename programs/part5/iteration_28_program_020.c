**Key features that target the uncovered lines:**

1. **Artificial/Built-in Declarations:**
   - Multiple `__builtin_*` functions used throughout
   - OpenMP pragma forces generation of parallel runtime helpers
   - `__attribute__((constructor))` creates initialization code
   - `__attribute__((weak))` may create artificial weak symbols

2. **Visibility Attributes:**
   - Explicit `visibility("hidden")` on multiple functions
   - Combination with `always_inline`, `weak`, `constructor` attributes
   - Mix of `visibility("default")` and `visibility("hidden")` functions

3. **Complex Control Flow:**
   - Loops with `__builtin_unpredictable()` conditions
   - `__builtin_trap()` and `__builtin_unreachable()` in conditional paths
   - `__attribute__((hot))` on vector function
   - `__builtin_assume_aligned()` for pointer optimization

4. **Vectorization:**
   - GCC vector types with explicit vector sizes
   - `__builtin_shuffle()` and `__builtin_convertvector()` operations
   - Vector arithmetic that may trigger auto-vectorization

5. **Sanitizer Interaction:**
   - Array bounds access in loops
   - Dynamic memory allocation
   - Compatible with `-fsanitize=address`

**Compilation options to maximize coverage:**
