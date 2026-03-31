omp_depend_t obj;

// Initialize the dependency object
#pragma omp depobj(obj) depend(inout: x)

// Use it in tasks
#pragma omp task depend(depobj: obj)
{
    // Task code
}

// Wait for tasks to complete
#pragma omp taskwait

// Destroy the dependency object when no longer needed
#pragma omp depobj(obj) destroy
