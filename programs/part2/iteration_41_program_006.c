## When to Use This Pattern

This is ideal for:
- Regular grids or matrices (image processing, stencil computations)
- When both coarse-grained (across teams) and fine-grained (SIMD) parallelism are needed
- GPU offloading with data-parallel operations

## Alternatives

If the conditional causes too much divergence:
