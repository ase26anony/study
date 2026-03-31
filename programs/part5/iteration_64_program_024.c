/* test_omp_depend_coverage.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test_omp_depend_coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare test variables of various types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    void *depobj_var = NULL;
    
    /* Block 1: Basic depend clause types in simple tasks */
    if (0) {
        /* OMP_CLAUSE_DEPEND_IN */
        #pragma omp task depend(in: x)
        { x = 1; }
        
        /* OMP_CLAUSE_DEPEND_OUT */
        #pragma omp task depend(out: y)
        { y = 2; }
        
        /* OMP_CLAUSE_DEPEND_INOUT */
        #pragma omp task depend(inout: z)
        { z = x + y; }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y) depend(out: z)
        { z = x + y; }
    }
    
    /* Block 2: Set-based dependency types */
    if (0) {
        /* OMP_CLAUSE_DEPEND_INOUTSET */
        #pragma omp task depend(inoutset: arr[0])
        { arr[0] = 10; }
        
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: arr[1])
        { arr[1] = 20; }
        
        /* Combined set dependencies */
        #pragma omp task depend(inoutset: arr[0], arr[1]) depend(out: arr[2])
        { arr[2] = arr[0] + arr[1]; }
    }
    
    /* Block 3: depobj dependency type */
    if (0) {
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        { x = 100; }
        
        /* depobj with array element */
        #pragma omp task depend(depobj: arr[3])
        { y = 200; }
    }
    
    /* Block 4: Complex dependency expressions */
    if (0) {
        /* Pointer dereference */
        #pragma omp task depend(out: *ptr)
        { *ptr = 42; }
        
        /* Array section (iterator modifier) - C/C++ syntax */
        #pragma omp task depend(in: arr[0:5])
        { 
            int sum = 0;
            for (int i = 0; i < 5; i++) sum += arr[i];
            x = sum;
        }
        
        /* Mixed complex dependencies */
        #pragma omp task depend(in: arr[0:3]) depend(out: *ptr) depend(inoutset: arr[5])
        {
            *ptr = arr[0] + arr[5];
        }
    }
    
    /* Block 5: Target regions with dependencies */
    if (0) {
        /* OMP_CLAUSE_DEPEND_IN in target */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x = x * 2; }
        
        /* OMP_CLAUSE_DEPEND_OUT in target */
        #pragma omp target depend(out: arr[0]) map(tofrom: arr[0])
        { arr[0] = 99; }
        
        /* Multiple dependencies in target */
        #pragma omp target depend(in: x) depend(out: y) map(tofrom: x, y)
        { y = x; x = 0; }
    }
    
    /* Block 6: Nested constructs for complex printing paths */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Creates task dependency graph */
                #pragma omp task depend(out: x)
                { x = 1; }
                
                #pragma omp task depend(in: x) depend(out: y)
                { y = x + 1; }
                
                #pragma omp task depend(in: y) depend(mutexinoutset: z)
                { z = y * 2; }
                
                #pragma omp task depend(inoutset: z) depend(depobj: depobj_var)
                { depobj_var = &z; }
            }
        }
    }
    
    /* Block 7: Combined parallel constructs with dependencies */
    if (0) {
        /* Combined target parallel with dependency */
        #pragma omp target parallel for depend(in: arr[0]) map(tofrom: arr[0:10])
        for (int i = 0; i < 10; i++) {
            arr[i] = i;
        }
        
        /* Parallel sections with task dependencies inside */
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                #pragma omp task depend(out: x)
                { x = 100; }
            }
            
            #pragma omp section
            {
                #pragma omp task depend(in: x) depend(out: y)
                { y = x * 2; }
            }
        }
    }
    
    /* Block 8: Multiple depend clauses to trigger OMP_CLAUSE_DEPEND_LAST iteration */
    if (0) {
        /* This should create a clause list where the pretty-printer iterates
         * through all clauses, potentially reaching the "last" marker */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[0]) depend(mutexinoutset: arr[1]) \
                         depend(depobj: depobj_var)
        {
            /* Complex operation using all dependencies */
            z = x + y + arr[0] + arr[1];
            depobj_var = &z;
        }
    }
    
    /* Block 9: Taskwait with depend clauses */
    if (0) {
        #pragma omp task depend(out: x)
        { x = 10; }
        
        #pragma omp task depend(in: x) depend(out: y)
        { y = x * 2; }
        
        /* Taskwait with dependency - tests different printing context */
        #pragma omp taskwait depend(inoutset: y)
        
        #pragma omp task depend(in: y)
        { z = y + 5; }
    }
    
    /* Block 10: Taskloop with dependencies */
    if (0) {
        #pragma omp taskloop depend(in: x) depend(out: arr[0:5]) grainsize(1)
        for (int i = 0; i < 5; i++) {
            arr[i] = x + i;
        }
    }
    
    return 0;
}
