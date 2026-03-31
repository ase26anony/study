omp_depend_t obj;
// ... some code that uses obj with task dependencies ...

// When done, destroy the dependency object
#pragma omp depobj(obj) destroy
