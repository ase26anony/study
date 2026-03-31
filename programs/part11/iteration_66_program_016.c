omp_depend_t dep_obj;
// Initialize/create the dependency object
#pragma omp depobj(dep_obj) update(inout)

// Use it in task dependencies
#pragma omp task depend(depobj: dep_obj)
{
    // Task code
}

// Destroy when done
#pragma omp depobj(dep_obj) destroy
