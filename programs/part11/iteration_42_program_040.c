#pragma omp task depend(in: x)      // Task reads x
#pragma omp task depend(out: y)     // Task writes y
#pragma omp task depend(inout: z)   // Task reads and writes z
