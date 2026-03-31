#include <omp.h>

int main() {
    omp_depend_t obj;
    
    // Create/initialize the dependency object
    #pragma omp depobj(obj) depend(inout: some_variable)
    
    // ... use the dependency object in tasks ...
    
    // Destroy the dependency object when no longer needed
    #pragma omp depobj(obj) destroy
    
    return 0;
}
