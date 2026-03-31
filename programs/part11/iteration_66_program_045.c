omp_depend_t obj;
#pragma omp depobj(obj) depend(in: x)  // Initialize with dependency

#pragma omp task depend(depobj: obj)
{
    // Task that depends on the dependency object
}

#pragma omp depobj(obj) destroy  // Explicit cleanup
