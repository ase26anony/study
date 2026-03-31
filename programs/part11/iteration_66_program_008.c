omp_depend_t obj;
// Initialize the dependency object
#pragma omp depobj(obj) depend(inout: x)

// Use it with tasks
#pragma omp task depend(depobj: obj)
{
    // Task code
}

// Later, destroy it when no longer needed
#pragma omp depobj(obj) destroy
