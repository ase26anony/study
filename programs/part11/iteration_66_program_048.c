omp_depend_t obj;

// Create/initialize the dependency object
#pragma omp depobj(obj) update(inout)

// Use it in tasks
#pragma omp task depend(depobj:obj)
{
    // Task code
}

// Later, destroy it
#pragma omp depobj(obj) destroy
