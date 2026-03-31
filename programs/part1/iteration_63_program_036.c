**Key features that target the uncovered lines:**

1. **Caller-Save Context Creation:**
   - `clobber_registers()` uses inline assembly with extensive clobber lists to force caller-saved register preservation
   - `__attribute__((noinline))` prevents inlining, creating clear call boundaries
   - Multiple live variables across function calls exceed available callee-saved registers

2. **Basic Block Structure Manipulation:**
   - `test_case_2` has a loop with function calls inside, creating blocks where restores are needed
   - `test_case_3` uses a switch statement creating multiple control flow edges
   - Conditional branches (`if-else`) create predecessor/successor blocks

3. **Register Pressure and Clobbering:**
   - Many local variables of different types (`int`, `float`, `double`, `long long`)
   - `volatile` variables prevent optimization elimination
   - Sequential calls to functions using different register subsets

4. **Compiler Optimization Interaction:**
   - Mixed `__attribute__((optimize("O3")))` and `O2` levels
   - Dependent arithmetic chains that benefit from instruction scheduling
   - `__builtin_unreachable()` to affect block termination analysis

5. **Edge Cases:**
   - `setjmp`/`longjmp` with variables that must survive
   - Mixed float/int operations engaging different register banks
   - External function calls forcing specific calling conventions

**Recommended compilation:**
