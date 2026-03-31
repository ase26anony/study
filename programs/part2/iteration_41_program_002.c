## Why This Matters:

1. **Memory Access Pattern**: C/C++ uses row-major order, so `array[i][j]` is contiguous in `j`
2. **SIMD Efficiency**: Vectorization needs contiguous memory for best performance
3. **GPU Parallelism**: `teams distribute` works best on the largest dimension

## Additional Optimizations:
