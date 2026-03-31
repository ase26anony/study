**Key features that target the uncovered lines:**

1. **Multiple TM Constructs**: Uses both `__transaction_atomic` and `__transaction_relaxed` blocks with `__transaction_cancel` for retry logic.

2. **Varied Loop Patterns**:
   - Simple array processing loops with load/store operations
   - Nested loops accessing multi-dimensional data
   - While loops with variable increments
   - Mixed transaction nesting (atomic inside relaxed)

3. **Non-constant Loop Bounds**: Uses function parameters and volatile variables to prevent compile-time optimization and loop unrolling.

4. **Shared Volatile Variables**: Global arrays marked as `volatile` to ensure loads/stores aren't optimized away.

5. **Function Attributes**: `noinline` and `noipa` prevent inter-procedural optimization that might eliminate TM constructs.

6. **Complex Control Flow**: Conditional transactions, retry logic, and mixed transaction types create diverse patterns for the TM transformation hook.

**Compilation options to maximize coverage**:
