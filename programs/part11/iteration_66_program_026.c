omp_depend_t obj;

// Create/initialize the dependency object
#pragma omp depobj(obj) update(inout(x))

// ... tasks that use the dependency ...

// Destroy when no longer needed
#pragma omp depobj(obj) destroy
