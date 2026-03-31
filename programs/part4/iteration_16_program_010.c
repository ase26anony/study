## Key Points:
1. **Predictable access pattern**: Compilers/hardware can prefetch data
2. **Cache-friendly**: Sequential access maximizes cache line utilization
3. **Pointer arithmetic**: `data[i]` compiles to `*(data + i)`

## Optimization Potential:
- **Auto-vectorization**: Compilers can use SIMD instructions
- **Loop unrolling**: Multiple iterations per loop iteration
- **Software prefetching**: Hint to load upcoming cache lines

## Modern Compiler Output:
With optimizations (`-O3`), compilers often transform this to pointer arithmetic:
