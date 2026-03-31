omp_depend_t dep_obj;
// Initialize the dependency object
#pragma omp depobj(dep_obj) depend(inout: x)

// Use it in tasks...

// Clean up when done
#pragma omp depobj(dep_obj) destroy
