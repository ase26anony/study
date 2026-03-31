omp_depend_t obj;
#pragma omp depobj(obj) depend(in: x)  // Create with dependency
// ... use obj in task dependencies ...
#pragma omp depobj(obj) destroy        // Explicitly destroy
