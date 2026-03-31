#pragma omp task depend(inout: x)  // Would correspond to OMP_CLAUSE_DEPEND_INOUT
#pragma omp task depend(update: mutexinoutset: ptr)  // Would correspond to OMP_CLAUSE_DEPEND_MUTEXINOUTSET
