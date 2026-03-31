omp_depend_t dep_obj;

// Initialize/create the dependency object
#pragma omp depobj(dep_obj) depend(inout: x)

// Use it in tasks
#pragma omp task depend(depobj: dep_obj)
{
    // Task code
}

// Destroy when no longer needed
#pragma omp depobj(dep_obj) destroy
