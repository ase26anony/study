omp_depend_t dep_obj;
// ... use dep_obj in task dependencies ...

// When done, destroy it to free resources
#pragma omp depobj(dep_obj) destroy
