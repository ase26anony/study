This program creates the specific loop patterns needed:

1. **Proper subset relationships**: Inner loops fully contained within outer loops (like Loop A inside the outer loop in `test_powerpc_nested_loops`).

2. **Intersecting but non-subset loops**: Loops D and E in the PowerPC test intersect but each has blocks outside the other.

3. **Disjoint loops**: Loops F and G in separate scopes with no block intersection.

4. **Complex control flow**: Uses `goto` for multiple entry points, `switch` statements inside loops, early exits with `break` and `return`, and computed gotos.

5. **Architecture targeting**: Uses `__attribute__((target(...)))` for PowerPC and ARM backends.

6. **Hardware loop candidates**: Includes counted loops with constant bounds, array operations, and optimization barriers.

To compile and test:
