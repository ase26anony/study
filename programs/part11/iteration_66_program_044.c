#include <omp.h>

int main() {
    omp_depend_t obj;
    int var = 0;
    
    // Create a dependency object
    #pragma omp depobj(obj) depend(inout: var)
    
    // Use it in tasks
    #pragma omp task depend(depobj: obj)
    {
        var = 42;
    }
    
    // Destroy when done
    #pragma omp depobj(obj) destroy
    
    return 0;
}
