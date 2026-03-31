3. **Memory Access Pattern**: Sequential access to arrays is cache-friendly.

## Potential Issues:

1. **Dependency on `sum`**: The accumulation creates a loop-carried dependency, which can limit SIMD performance. Consider:
