/* test-omp-depend-coverage.c
 * 
 * This test program is designed to trigger GCC's tree pretty-printer
 * for all OMP_CLAUSE_DEPEND_* enumeration values in tree-pretty-print.cc.
 * Compile with flags like:
 *   -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple
 *   -O2 -fopenmp -fdump-tree-optimized
 *   -O0 -fopenmp -foffload=disable -fdump-tree-omplower
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Prevent runtime execution while ensuring compilation */
    if (0) {
        /* Block 1: Basic depend clause types in tasks */
        /* OMP_CLAUSE_DEPEND_IN */
        #pragma omp task depend(in: x)
        { x = 1; }
        
        /* OMP_CLAUSE_DEPEND_OUT */
        #pragma omp task depend(out: y)
        { y = 2; }
        
        /* OMP_CLAUSE_DEPEND_INOUT */
        #pragma omp task depend(inout: z)
        { z = x + y; }
        
        /* OMP_CLAUSE_DEPEND_INOUTSET */
        #pragma omp task depend(inoutset: arr[0])
        { arr[0] = 10; }
        
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: arr[1])
        { arr[1] = 20; }
        
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 30; }
    }
    
    if (0) {
        /* Block 2: Multiple depend clauses on single construct 
           This helps trigger iteration through all clause types,
           potentially reaching OMP_CLAUSE_DEPEND_LAST during printing */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[0]) depend(mutexinoutset: arr[1]) \
                         depend(depobj: depobj_var)
        {
            x = y + z;
            arr[0] = arr[1] + depobj_var;
        }
    }
    
    if (0) {
        /* Block 3: Complex dependency expressions */
        int *dynamic_ptr = malloc(sizeof(int));
        
        /* Array element with index */
        #pragma omp task depend(in: arr[2])
        { arr[2] = 100; }
        
        /* Pointer dereference */
        #pragma omp task depend(out: *dynamic_ptr)
        { *dynamic_ptr = 200; }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y, z)
        { int sum = x + y + z; }
        
        free(dynamic_ptr);
    }
    
    if (0) {
        /* Block 4: Target regions with depend clauses */
        /* OMP_CLAUSE_DEPEND_IN in target */
        #pragma omp target depend(in: x) map(to: x)
        { x = x + 1; }
        
        /* OMP_CLAUSE_DEPEND_OUT in target */
        #pragma omp target depend(out: arr[3]) map(from: arr[3])
        { arr[3] = 300; }
        
        /* OMP_CLAUSE_DEPEND_INOUT in target */
        #pragma omp target depend(inout: arr[4]) map(tofrom: arr[4])
        { arr[4] *= 2; }
    }
    
    if (0) {
        /* Block 5: Nested constructs for complex printing paths */
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Tasks with various depend types inside parallel region */
                #pragma omp task depend(inoutset: y)
                { y = 40; }
                
                #pragma omp task depend(mutexinoutset: z)
                { z = 50; }
                
                #pragma omp task depend(depobj: x)
                { x = 60; }
            }
        }
    }
    
    if (0) {
        /* Block 6: Combined constructs */
        #pragma omp target parallel for depend(in: arr[5]) map(tofrom: arr[5])
        for (int i = 0; i < 10; i++) {
            arr[5] += i;
        }
    }
    
    if (0) {
        /* Block 7: Iterator modifier (C/C++ specific) */
        int len = 10;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i * 10;
            }
        }
    }
    
    /* Ensure variables are used to avoid warnings */
    (void)x; (void)y; (void)z;
    (void)arr; (void)ptr; (void)depobj_var;
    
    return 0;
}
