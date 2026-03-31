omp_depend_t obj;
// Initialize/create the dependency object
#pragma omp depobj(obj) depend(inout: x)

// Use it in tasks...

// When done, destroy it
#pragma omp depobj(obj) destroy
