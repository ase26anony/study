## Key Points:
1. **Sequential Access**: The loop accesses `data[0]`, `data[1]`, `data[2]`, ... in order
2. **Cache-friendly**: This pattern is optimal for CPU caches due to spatial locality
3. **Pointer Arithmetic**: `data[i]` is equivalent to `*(data + i)`
4. **Post-increment**: The increment of `i` happens at the end of each iteration

## Performance Considerations:
- **Prefetching**: Modern CPUs can detect this sequential pattern and prefetch upcoming cache lines
- **Vectorization**: Compilers may auto-vectorize this to use SIMD instructions (SSE/AVX)
- **Loop unrolling**: Compilers may unroll the loop to reduce branch overhead

## Alternative Pointer-based Version:
