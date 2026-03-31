#pragma omp task depend(in: x)      // Reads x
#pragma omp task depend(out: y)     // Writes y  
#pragma omp task depend(inout: z)   // Reads and writes z
