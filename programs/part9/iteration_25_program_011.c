## Key Features of This Test Program:

1. **Comprehensive `depend` Clause Coverage**:
   - Basic types: `in`, `out`, `inout`
   - OpenMP 5.0+ types with `update` modifier: `update(in)`, `update(out)`, `update(inout)`, `update(mutexinoutset)`, `update(inoutset)`
   - OpenMP 5.2+ `destroy` clause

2. **C++-Specific Features**:
   - References (`int& ref_a`)
   - Class objects (`MyObj`)
   - Pointers to objects (`&obj`)

3. **Multiple OpenMP Contexts**:
   - Tasks within parallel region
   - `taskgroup` for nested context
   - `target` construct (if OpenMP 5.0+ supported)

4. **OpenMP Version Guards**:
   - `#if _OPENMP >= 201811` for OpenMP 5.0+ features
   - Graceful degradation for older OpenMP versions

5. **Valid Dependency Graph**:
   - Tasks have actual data dependencies
   - `taskwait` ensures completion
   - Deterministic output

## Compilation and Testing:
