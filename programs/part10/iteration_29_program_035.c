#pragma omp task depend(in: x)          // OMP_CLAUSE_DEPEND_IN
#pragma omp task depend(inout: x)       // OMP_CLAUSE_DEPEND_INOUT  
#pragma omp task depend(out: x)         // OMP_CLAUSE_DEPEND_OUT
#pragma omp task depend(mutexinoutset: x) // OMP_CLAUSE_DEPEND_MUTEXINOUTSET
