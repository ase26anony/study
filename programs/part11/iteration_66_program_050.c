  omp_depend_t obj;
  #pragma omp depobj(obj) depend(inout: var)  // Initialize with dependency
  #pragma omp task depend(depobj: obj)        // Use in task
  #pragma omp depobj(obj) destroy             // Clean up
