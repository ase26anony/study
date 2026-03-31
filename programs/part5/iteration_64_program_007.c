/* test-omp-depend-coverage.c
 * 
 * This test program is designed to trigger GCC's tree pretty-printer
 * for all OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting
 * the uncovered lines in tree-pretty-print.cc (lines 824-846).
 * 
 * Compile with: 
 *   gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * Additional flags for more dumps:
 *   -fdump-tree-original -fdump-tree-optimized -foffload=disable
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int len = 10;
    
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
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y) depend(out: z)
        { z = x + y; }
    }
    
    if (0) {
        /* Block 2: Set-based dependency types */
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: x)
        { x = x * 2; }
        
        /* OMP_CLAUSE_DEPEND_INOUTSET */
        #pragma omp task depend(inoutset: y)
        { y = y + 1; }
        
        /* Combined set types in nested parallel region */
        #pragma omp parallel
        {
            #pragma omp task depend(mutexinoutset: x)
            { x = 1; }
            #pragma omp task depend(inoutset: y)
            { y = 2; }
        }
    }
    
    if (0) {
        /* Block 3: DEPOBJ dependency type */
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: ptr)
        { *ptr = 42; }
        
        /* depobj with array element */
        int *ptr2 = &arr[0];
        #pragma omp task depend(depobj: ptr2)
        { arr[0] = 99; }
    }
    
    if (0) {
        /* Block 4: Complex dependency expressions */
        /* Array element dependency */
        #pragma omp task depend(in: arr[0])
        { arr[0] = 100; }
        
        /* Pointer dereference */
        #pragma omp task depend(out: *ptr)
        { *ptr = 200; }
        
        /* Iterator modifier (C/C++ specific) */
        #pragma omp task depend(in: arr[0:len])
        { 
            for (int i = 0; i < len; i++) 
                arr[i] = i; 
        }
    }
    
    if (0) {
        /* Block 5: Target regions with dependencies */
        /* OMP_CLAUSE_DEPEND_IN in target region */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x = x * 3; }
        
        /* OMP_CLAUSE_DEPEND_OUT in target region */
        #pragma omp target depend(out: arr) map(tofrom: arr[0:10])
        { arr[0] = 999; }
        
        /* Combined construct */
        #pragma omp target parallel for depend(inout: x)
        for (int i = 0; i < 10; i++) {
            arr[i] = x + i;
        }
    }
    
    if (0) {
        /* Block 6: Multiple depend clauses to trigger iteration 
           through OMP_CLAUSE_DEPEND_LAST */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(mutexinoutset: arr[0]) depend(depobj: ptr)
        {
            x = y + z;
            *ptr = arr[0];
        }
    }
    
    /* Additional test: Nested tasks with complex dependencies */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp task depend(inoutset: x)
            {
                #pragma omp task depend(in: x)
                { y = x + 1; }
                
                #pragma omp task depend(out: z)
                { z = x * 2; }
            }
        }
    }
    
    /* Force a taskwait to ensure dependencies are analyzed */
    #pragma omp taskwait
    
    return 0;
}
