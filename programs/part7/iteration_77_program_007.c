**Key Design Elements:**

1. **Multiple TM Loop Patterns:**
   - `tm_loop_transform1`: Simple for-loops with array accesses and volatile variables
   - `tm_loop_transform2`: Nested loops with relaxed transactions and while-loops
   - `tm_loop_transform3`: Conditional TM with potential transaction cancellation
   - `tm_nested_transactions`: Recursive TM with nested transactions in loops
   - `tm_pointer_arithmetic`: Pointer arithmetic loops likely to trigger load/store transformations

2. **Non-Constant Loop Bounds:**
   - All loops use function parameters or global variables for bounds
   - Prevents compile-time unrolling that might bypass TM transformations

3. **Volatile and Shared Variables:**
   - `g_shared_counter` marked volatile to prevent optimization
   - Global arrays accessed from multiple TM regions
   - Volatile pointer operations in `tm_pointer_arithmetic`

4. **Transaction Variants:**
   - Both `__transaction_atomic` and `__transaction_relaxed` used
   - Transaction cancellation in `tm_loop_transform3`
   - Nested transactions in `tm_nested_transactions`

5. **Anti-Optimization Measures:**
   - `__attribute__((noinline, noipa))` prevents function inlining
   - Final checksum calculation ensures all code executes
   - Output to stdout prevents dead code elimination

**Compilation and Testing:**
