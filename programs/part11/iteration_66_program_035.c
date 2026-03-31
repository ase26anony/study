omp_depend_t dep1, dep2;

// Initialize dependency objects
#pragma omp depobj(dep1) depend(in: x)
#pragma omp depobj(dep2) depend(out: y)

// Use them in tasks
#pragma omp task depend(depobj: dep1, dep2)
{
    // Task code here
}

// Later, destroy them when no longer needed
#pragma omp depobj(dep1) destroy
#pragma omp depobj(dep2) destroy
