omp_depend_t obj;
#pragma omp depobj(obj) depend(in: some_variable)  // Initialize with dependency

// ... tasks that use this dependency ...

#pragma omp depobj(obj) destroy  // Explicit cleanup
