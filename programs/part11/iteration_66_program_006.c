omp_depend_t dep_obj;

// Initialize and use the dependency object
#pragma omp depobj(dep_obj) update(inout)

// ... tasks using the dependency ...

// Explicitly destroy when done
#pragma omp depobj(dep_obj) destroy

// Can now reuse the variable if needed
