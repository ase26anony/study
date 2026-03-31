omp_depend_t obj;

// Create a dependency object
#pragma omp depobj(obj) depend(inout: x)

// Use it in tasks
#pragma omp task depend(depobj: obj)
{
    // Task code
}

// Destroy when no longer needed
#pragma omp depobj(obj) destroy
