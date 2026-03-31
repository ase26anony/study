#pragma omp task depend(in: x)      // update(in)
#pragma omp task depend(out: y)     // update(out)
#pragma omp task depend(inout: z)   // update(inout)
