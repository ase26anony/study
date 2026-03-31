#include <omp.h>

int main() {
    omp_depend_t dep_obj;
    
    // Initialize the dependency object
    #pragma omp depobj(dep_obj) depend(inout: x)
    
    // ... use dep_obj in tasks ...
    
    // Destroy the dependency object when done
    #pragma omp depobj(dep_obj) destroy
    
    return 0;
}
