/* test-depend-clauses.c
 * 
 * This test program is designed to trigger GCC's tree pretty-printer
 * for all OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting
 * the uncovered lines in tree-pretty-print.cc (lines 824-846).
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test-depend-clauses.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency tests */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Prevent actual execution while ensuring compilation */
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
        { arr[0] = 5; }
        
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: arr[1])
        { arr[1] = 6; }
        
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 7; }
    }
    
    if (0) {
        /* Block 2: Multiple depend clauses on single construct 
           This helps trigger iteration through all clause types,
           potentially reaching OMP_CLAUSE_DEPEND_LAST */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[0]) depend(mutexinoutset: arr[1])
        {
            z = x + y;
            arr[0] = z;
            arr[1] = z + 1;
        }
    }
    
    if (0) {
        /* Block 3: Target regions with depend clauses */
        /* OMP_CLAUSE_DEPEND_IN with array element */
        #pragma omp target depend(in: arr[2])
        { arr[2] = 10; }
        
        /* OMP_CLAUSE_DEPEND_OUT with pointer dereference */
        #pragma omp target depend(out: *ptr)
        { *ptr = 20; }
        
        /* Combined target parallel with depend */
        #pragma omp target parallel for depend(inout: arr[3])
        for (int i = 0; i < 10; i++) {
            arr[3] += i;
        }
    }
    
    if (0) {
        /* Block 4: Nested parallel region with task dependencies */
        #pragma omp parallel
        {
            /* OMP_CLAUSE_DEPEND_INOUTSET in nested context */
            #pragma omp task depend(inoutset: y)
            { y = y * 2; }
            
            /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET in nested context */
            #pragma omp task depend(mutexinoutset: z)
            { z = z + 1; }
            
            /* Multiple items in single clause */
            #pragma omp task depend(in: x, y) depend(out: z, arr[4])
            {
                z = x + y;
                arr[4] = z;
            }
        }
    }
    
    if (0) {
        /* Block 5: Iterator modifier (C/C++ specific) */
        int len = 5;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i;
            }
        }
    }
    
    if (0) {
        /* Block 6: Complex dependency chain to test pretty-printer thoroughly */
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 1; }
        
        #pragma omp task depend(in: depobj_var) depend(out: x)
        { x = depobj_var * 2; }
        
        #pragma omp task depend(in: x) depend(inoutset: y) depend(mutexinoutset: z)
        {
            y = x + 1;
            z = x + 2;
        }
        
        #pragma omp task depend(in: y, z) depend(out: arr[5])
        { arr[5] = y + z; }
    }
    
    /* Ensure variables are used to avoid warnings */
    (void)x; (void)y; (void)z;
    (void)arr; (void)ptr; (void)depobj_var;
    
    return 0;
}
