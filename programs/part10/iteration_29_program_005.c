#pragma omp task depend(in: x)          // OMP_CLAUSE_DEPEND_IN
#pragma omp task depend(inout: y)       // OMP_CLAUSE_DEPEND_INOUT  
#pragma omp task depend(out: z)         // OMP_CLAUSE_DEPEND_OUT
