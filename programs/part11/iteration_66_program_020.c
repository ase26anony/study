omp_depend_t obj;
#pragma omp depobj(obj) depend(in: x)  // Initialize with dependency
// ... tasks using obj for dependencies ...
#pragma omp depobj(obj) destroy        // Clean up when done
