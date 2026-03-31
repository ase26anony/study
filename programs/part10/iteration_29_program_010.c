#pragma omp task depend(in: x)      // Task depends on x being available for reading
#pragma omp task depend(out: y)     // Task produces y
#pragma omp task depend(inout: z)   // Task reads and writes z
