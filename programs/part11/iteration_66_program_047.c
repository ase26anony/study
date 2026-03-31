omp_depend_t obj;

// Initialize the dependency object
#pragma omp depobj(obj) depend(in: x)

// Use it in tasks
#pragma omp task depend(depobj: obj)
{
    // Task code
}

// Destroy when no longer needed
#pragma omp depobj(obj) destroy
