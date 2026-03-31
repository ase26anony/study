omp_depend_t obj;

// Initialize the dependency object
#pragma omp depobj(obj) depend(inout: x)

// Use the dependency object in tasks
#pragma omp task depend(depobj: obj)
{
    // Task code
}

// Destroy when no longer needed
#pragma omp depobj(obj) destroy
