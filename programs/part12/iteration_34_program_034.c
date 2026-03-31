## Key Features:

1. **Exhaustive `depend` Modifiers**: The program uses all `depend` clause modifiers:
   - `depend(in: ...)` - lines 28-29
   - `depend(out: ...)` - lines 34-35 (with deprecated variable)
   - `depend(inout: ...)` - lines 40-41
   - `depend(mutexinoutset: ...)` - lines 46-47
   - `depend(inoutset: ...)` - lines 52-53
   - `depend(destroy: ...)` - lines 96-97

2. **Diagnostic Triggers**:
   - Uses `[[deprecated]]` variable (line 8) in task dependency (line 35)
   - Compile with `-Wdeprecated-declarations` to see warnings

3. **Compiler Dump Compatibility**:
   - Uses complex nested structures (templates, classes, namespaces)
   - Multiple parallel regions and task directives
   - Compile with `-fdump-tree-original` to see pretty-printed output

4. **Template and `constexpr` Contexts**:
   - Template function `process_with_depend` (line 20)
   - Template specialization (line 60)
   - `if constexpr` usage (line 83)
   - Recursive template instantiation (line 87)

5. **Structural Diversity**:
   - Namespace `OpenMPTest` (line 19)
   - Class `TaskManager` (line 75)
   - Constructor attribute function (line 105)
   - Lambda expression (line 119)

6. **Execution Flow**:
   - Initializes shared data structure
   - Calls template functions with different depend modifiers
   - Uses lambda with captured dependencies
   - Final cleanup with `depend(destroy: ...)`
   - Calculates checksum to prevent dead code elimination

## Compilation Commands:
