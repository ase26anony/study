// Good pattern for GPU offloading:
// - Outer loop parallelized across GPU threads/teams
// - Inner loop vectorized within each thread

// May be suboptimal if:
// - M is very small (< 16) - SIMD won't be effective
// - N is very small - not enough work for GPU teams
