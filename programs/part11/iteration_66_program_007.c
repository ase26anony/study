  omp_depend_t dep;
  #pragma omp depobj(dep) depend(inout: x)  // Initialize with dependency
  // ... tasks using 'dep' ...
  #pragma omp depobj(dep) destroy           // Clean up
