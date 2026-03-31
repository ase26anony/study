omp_depend_t obj;

// Initialize the dependency object
#pragma omp depobj(obj) depend(in: x)

// Use it in tasks
#pragma omp task depend(depobj: obj)
{
    // Task code
}

// When done, destroy it
#pragma omp depobj(obj) destroy
