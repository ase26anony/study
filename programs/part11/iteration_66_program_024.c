omp_depend_t obj;
#pragma omp depobj(obj) depend(inout: x)  // Create with dependency
// ... use obj in task dependencies ...
#pragma omp depobj(obj) destroy           // Clean up when done
