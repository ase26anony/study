#pragma omp task depend(in: x)           // OMP_CLAUSE_DEPEND_IN
#pragma omp task depend(out: y)          // OMP_CLAUSE_DEPEND_OUT  
#pragma omp task depend(inout: z)        // OMP_CLAUSE_DEPEND_INOUT
