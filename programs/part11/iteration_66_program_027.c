omp_depend_t dep_obj;

// Create and use the dependency object
#pragma omp depobj(dep_obj) update(inout)

// ... use dep_obj in tasks ...

// When done, destroy it
#pragma omp depobj(dep_obj) destroy
