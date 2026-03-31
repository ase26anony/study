omp_depend_t obj;
#pragma omp depobj(obj) depend(in: x)  // Initialize with dependency
// ... tasks that use this dependency ...
#pragma omp depobj(obj) destroy        // Clean up
