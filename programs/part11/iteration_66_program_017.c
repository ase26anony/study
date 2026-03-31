omp_depend_t obj;
// Initialize the dependency object
#pragma omp depobj(obj) depend(inout: x)

// Use it in tasks
#pragma omp task depend(depobj: obj)
{
    // Task code
}

// Destroy when done
#pragma omp depobj(obj) destroy
