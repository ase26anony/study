omp_depend_t dep;
#pragma omp depobj(dep) depend(in: x)  // Initialize with dependency
// ... use dep in task dependencies ...
#pragma omp depobj(dep) destroy        // Clean up when done
