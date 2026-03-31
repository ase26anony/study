omp_depend_t dep_obj;

// Initialize the dependency object
#pragma omp depobj(dep_obj) depend(inout: x)

// Use it in tasks
#pragma omp task depend(depobj: dep_obj)
{
    // Task code
}

// Destroy when done
#pragma omp depobj(dep_obj) destroy
