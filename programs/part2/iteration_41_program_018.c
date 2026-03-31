// Parallel execution hierarchy:
// 1. Outer loop (j) distributed across teams on target device
// 2. Inner loop (i) vectorized with SIMD instructions
//    - Each SIMD lane processes multiple i iterations concurrently
//    - Vector length up to 16 (or hardware maximum if smaller)
