**Key design elements that target the uncovered code:**

1. **Explicit register variable**: `register int reg_var asm ("r12")` binds to a call-clobbered register on x86_64.

2. **Function calls at block boundaries**: 
   - `return critical;` immediately after `helper1()` call in `test_block_end()`
   - `break;` after `helper2()` call in switch case
   - Loop back edges after calls in `test_switch()`

3. **Register pressure**: Multiple local variables (`v1`-`v8`, `a`-`h`) compete for registers.

4. **Control flow complexity**: Multiple `if/else` branches, loops, and switch statements create many basic blocks.

5. **Live values across calls**: `reg_var`/`critical`/`key` are used before and after function calls.

6. **Anti-optimization**: `volatile` globals, `noinline`/`noclone` attributes, and result usage prevent call elimination.

**Compilation and verification:**
