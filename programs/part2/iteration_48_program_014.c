## Key Design Elements:

1. **Complete Coverage of All Cases**: The code includes:
   - `depend(update: in)` (line 31)
   - `depend(update: inout)` (line 32)
   - `depend(update: out)` (line 33)
   - `depend(update: mutexinoutset)` (line 34)
   - `depend(update: inoutset)` (line 35)
   - `depend(destroy: ...)` (lines 42, 58, 59, 94, 103)

2. **Triggering Diagnostics**:
   - Multiple `unused` variables (lines 29, 43, 81) will trigger `-Wunused-variable` warnings
   - Complex AST structures ensure the pretty-printer is invoked

3. **Multiple Contexts**:
   - Template function (lines 17-46)
   - Class member function (lines 49-64)
   - Lambda expression (lines 67-82)
   - Main function with various constructs

4. **Preventing Optimization**:
   - `volatile` external variables
   - Template instantiation with different types
   - `std::function` to preserve lambda
   - `#pragma omp declare target` for valid target variables

## Compilation Commands:
