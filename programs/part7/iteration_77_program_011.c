## Key Design Elements:

1. **Multiple TM Constructs**: Uses both `__transaction_atomic` and `__transaction_relaxed` blocks to engage GCC's TM system.

2. **Varied Loop Patterns**:
   - Simple array processing loops with load-store operations
   - Nested loops for multi-dimensional access
   - While loops and do-while loops
   - Loops with variable bounds (non-constant)

3. **Memory Access Patterns**:
   - Array indexing with arithmetic
   - Pointer-based access
   - Volatile variable access
   - Mixed load and store operations within loops

4. **TM-Specific Features**:
   - `__transaction_cancel` for retry logic
   - Nested transaction attempts
   - Conditional transaction execution

5. **Optimization Prevention**:
   - `noinline` and `noipa` attributes
   - Volatile variables for loop bounds
   - Checksum computation to ensure execution
   - Function arguments used as loop bounds

## Compilation and Testing:
