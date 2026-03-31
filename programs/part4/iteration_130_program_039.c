This test program systematically addresses all requirements:

1. **Exhaustive Unsigned Comparison Invocation**: The nested loops perform `ucmp` on all pairs of test cases, covering all four conditional branches.

2. **Boundary Value and Sign-Bit Manipulation**: Test cases include values with MSB set (negative signed but large unsigned) and without MSB, ensuring the unsigned cast is thoroughly tested.

3. **Volatile Control Flow**: Volatile variables are used for initial values, preventing compile-time constant folding.

4. **Mixed Signed/Unsigned Contexts**: Both `ucmp` and `scompare` are called on the same pairs, exercising different comparison semantics.

5. **Loop-Based Value Generation**: The nested loops generate many comparison instances, and additional random values are generated for the tree.

6. **Recursive Data Structures**: A binary tree using `ucmp` for comparisons is implemented and tested.

**Compilation recommendations:**
