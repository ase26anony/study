#pragma omp task depend(in: x)      // Task depends on x as input
#pragma omp task depend(out: y)     // Task produces y as output
#pragma omp task depend(inout: z)   // Task both reads and writes z
