omp_depend_t obj;
#pragma omp depobj(obj) depend(in: x)  // Initialize with a dependency
// ... tasks that use obj ...
#pragma omp depobj(obj) destroy        // Clean up when done
