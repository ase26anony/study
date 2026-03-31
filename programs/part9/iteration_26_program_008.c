## Key Features That Trigger gengtype Coverage:

1. **Comprehensive Type Coverage**:
   - All 10 type categories from the uncovered switch statement are represented
   - Multiple instances of each category with different qualifiers and contexts

2. **Deep Type Graphs**:
   - Mutually recursive structs (`MutualA` ↔ `MutualB`)
   - Self-referential pointers (`self_ptr`)
   - Nested Standard Library containers (`vector<vector<double>>`)

3. **C++-Specific Constructs**:
   - Class templates with multiple instantiations
   - Inheritance hierarchies (single and multiple)
   - Lambda expressions and `std::function`
   - `decltype` and `auto` type deduction

4. **Prevention of Optimization**:
   - `volatile` qualifiers on globals
   - `use()` function calls with addresses
   - `__attribute__((used))` on preservation variable
   - Constructor function that references globals

5. **Compiler Dump Triggers**:
   - File-scope declarations force type metadata generation
   - Complex template instantiations create intricate internal representations
   - Multiple translation unit-like behavior with `extern "C"` sections

## Recommended Compilation Commands:
