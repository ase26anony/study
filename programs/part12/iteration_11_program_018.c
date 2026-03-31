## Key Design Elements:

1. **Partial Basic Block Overlap**: Each test function creates loops where:
   - Inner loops share some basic blocks with outer loops
   - Outer loops have additional blocks not in inner loops (via `else` branches)
   - This triggers `bitmap_intersect_compl_p` checks

2. **Multiple Nesting Patterns**:
   - `test_nested_loops_partial_overlap()`: 3-level nesting with conditional inner loops
   - `test_sibling_loops()`: Two mutually exclusive inner loops (siblings)
   - `test_complex_nesting()`: Multiple inner loops at different nesting levels
   - `test_loop_with_break_continue()`: Loops with early exits creating complex CFG

3. **Side Effects & Anti-Optimization**:
   - `volatile` array writes prevent dead code elimination
   - `rand()` calls create unpredictable but bounded conditions
   - Checksum accumulation ensures all loops execute
   - Global array accesses with loop indices

4. **Compilation Recommendations**:
