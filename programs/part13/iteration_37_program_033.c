This program systematically exercises all partition codes through three approaches:

1. **Template Metaprogramming**: Generates OpenACC loops for partition codes 0-7 at compile time using `if constexpr`.

2. **Runtime Enumeration**: Tests codes -1 through 9 using volatile variables to prevent optimization, covering all valid codes (0-7) and illegal values.

3. **Complex Broadcasting**: Uses multi-dimensional arrays with mixed partitioning clauses to trigger the internal mapping function.

**Compilation commands:**
