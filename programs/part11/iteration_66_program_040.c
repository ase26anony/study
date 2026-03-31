omp_depend_t obj;
#pragma omp depobj(obj) depend(inout: x)  // Create with dependency
// ... parallel region using obj ...
#pragma omp depobj(obj) destroy           // Explicit destruction
