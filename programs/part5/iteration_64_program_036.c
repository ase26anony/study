/* test-omp-depend-coverage.c
 * 
 * This test program is designed to trigger GCC's tree pretty-printer
 * for all OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting
 * the uncovered block in tree-pretty-print.cc (lines 824-846).
 * 
 * Compile with: 
 *   gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * 
 * Additional flags for more dumps:
 *   -fdump-tree-original -fdump-tree-optimized -fdump-tree-all
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Prevent actual execution while ensuring compilation */
    if (0) {
        /* Block 1: Basic depend clause types in OpenMP tasks */
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
           potentially reaching OMP_CLAUSE_DEPEND_LAST context */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[0]) depend(mutexinoutset: arr[1])
        {
            x = y + z;
            arr[0] = arr[1];
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
        #pragma omp task depend(in: x, y, arr[3])
        { arr[3] = x + y; }
        
        free(dynamic_ptr);
    }
    
    if (0) {
        /* Block 4: depend clauses in target regions */
        #pragma omp target depend(in: x) depend(out: arr[4])
        { arr[4] = x * 2; }
        
        /* Combined construct */
        #pragma omp target parallel for depend(inout: arr[5])
        for (int i = 0; i < 10; i++) {
            arr[5] += i;
        }
    }
    
    if (0) {
        /* Block 5: Nested tasks with dependencies */
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(out: x)
                { x = 1; }
                
                #pragma omp task depend(in: x) depend(out: y)
                { y = x + 1; }
                
                #pragma omp task depend(in: y) depend(mutexinoutset: z)
                { z = y * 2; }
                
                #pragma omp task depend(inoutset: arr[6]) depend(in: z)
                { arr[6] = z; }
            }
        }
    }
    
    if (0) {
        /* Block 6: Iterator modifier (C/C++ specific) */
        int len = 10;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i;
            }
        }
    }
    
    if (0) {
        /* Block 7: depobj with pointer */
        #pragma omp task depend(depobj: ptr)
        {
            *ptr = 42;
        }
        
        /* Another depobj example with different variable */
        int another_depobj = 0;
        #pragma omp task depend(depobj: another_depobj)
        {
            another_depobj = 99;
        }
    }
    
    /* Ensure all constructs are processed by preventing dead code elimination */
    volatile int keep = 0;
    if (keep) {
        x = y + z + arr[0];
    }
    
    return 0;
}
