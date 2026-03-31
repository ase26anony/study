/* test-omp-depend-coverage.c
 * 
 * This test program is designed to trigger GCC's tree pretty-printer
 * for all OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting
 * the uncovered block in tree-pretty-print.cc (lines 824-846).
 *
 * Compile with: 
 *   gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * Additional flags for more dumps:
 *   -fdump-tree-optimized -fdump-tree-original -foffload=disable
 *
 * The program does not need to execute correctly; it's meant to be parsed
 * and transformed, triggering pretty-printing during compilation.
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x, y, z;
    int arr[100];
    int *ptr = &x;
    int len = 50;
    
    /* Prevent accidental execution of OpenMP regions */
    if (0) {
        /* Block 1: OMP_CLAUSE_DEPEND_IN */
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
        
        /* Block 2: OMP_CLAUSE_DEPEND_OUT */
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
        
        /* Block 3: OMP_CLAUSE_DEPEND_INOUT */
        #pragma omp task depend(inout: z)
        {
            z = z + 1;
        }
        
        /* Block 4: OMP_CLAUSE_DEPEND_INOUTSET */
        #pragma omp task depend(inoutset: arr[0])
        {
            arr[0] = 10;
        }
        
        /* Block 5: OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: arr[1])
        {
            arr[1] = 20;
        }
        
        /* Block 6: OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: ptr)
        {
            *ptr = 30;
        }
    }
    
    /* Block 7: Multiple depend clauses in single construct 
       (helps trigger iteration through all types, potentially reaching LAST) */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[0]) depend(mutexinoutset: arr[1]) \
                         depend(depobj: ptr)
        {
            x = y + z;
        }
    }
    
    /* Block 8: Target region with depend clause */
    if (0) {
        #pragma omp target depend(in: arr[0:10])
        {
            arr[0] = 100;
        }
    }
    
    /* Block 9: Parallel region containing tasks with dependencies */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp task depend(in: x)
            { x = 1; }
            
            #pragma omp task depend(out: y)
            { y = 2; }
            
            #pragma omp task depend(inout: z)
            { z = 3; }
        }
    }
    
    /* Block 10: Combined construct with depend */
    if (0) {
        #pragma omp target parallel for depend(out: arr[0:len])
        for (int i = 0; i < len; i++) {
            arr[i] = i;
        }
    }
    
    /* Block 11: Complex dependency expressions */
    if (0) {
        int *dyn_arr = malloc(100 * sizeof(int));
        
        #pragma omp task depend(in: arr[5]) depend(out: *ptr) \
                         depend(inout: dyn_arr[10:20])
        {
            dyn_arr[10] = arr[5] + *ptr;
        }
        
        free(dyn_arr);
    }
    
    /* Block 12: Iterator modifier (C/C++ specific) */
    if (0) {
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i * 2;
            }
        }
    }
    
    return 0;
}
