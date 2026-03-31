/* test-omp-depend-coverage.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer for
 * all OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting
 * the uncovered block in tree-pretty-print.cc lines 824-846.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Block 1: OMP_CLAUSE_DEPEND_IN */
    if (0) {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
    
    /* Block 2: OMP_CLAUSE_DEPEND_OUT */
    if (0) {
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
    }
    
    /* Block 3: OMP_CLAUSE_DEPEND_INOUT */
    if (0) {
        #pragma omp task depend(inout: z)
        {
            z++;
        }
    }
    
    /* Block 4: OMP_CLAUSE_DEPEND_INOUTSET */
    if (0) {
        #pragma omp task depend(inoutset: arr[0])
        {
            arr[0] = 10;
        }
    }
    
    /* Block 5: OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
    if (0) {
        #pragma omp task depend(mutexinoutset: arr[1])
        {
            arr[1] = 20;
        }
    }
    
    /* Block 6: OMP_CLAUSE_DEPEND_DEPOBJ */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 30;
        }
    }
    
    /* Block 7: Multiple depend clauses to potentially trigger OMP_CLAUSE_DEPEND_LAST iteration */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z)
        {
            x = y + z;
        }
    }
    
    /* Block 8: Complex dependency expressions */
    if (0) {
        int *dynamic_ptr = malloc(sizeof(int));
        #pragma omp task depend(in: *ptr) depend(out: *dynamic_ptr) depend(in: arr[2])
        {
            *dynamic_ptr = *ptr + arr[2];
        }
        free(dynamic_ptr);
    }
    
    /* Block 9: Target region with depend clause */
    if (0) {
        #pragma omp target depend(inout: x)
        {
            x *= 2;
        }
    }
    
    /* Block 10: Combined construct */
    if (0) {
        #pragma omp target parallel for depend(out: arr[0:5])
        for (int i = 0; i < 5; i++) {
            arr[i] = i;
        }
    }
    
    /* Block 11: Nested tasks in parallel region */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(inoutset: y)
                { y = 1; }
                
                #pragma omp task depend(mutexinoutset: z)
                { z = 2; }
                
                #pragma omp task depend(in: y) depend(out: x)
                { x = y + z; }
            }
        }
    }
    
    /* Block 12: Iterator modifier (C/C++ specific) */
    if (0) {
        int len = 5;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] += 1;
            }
        }
    }
    
    /* Block 13: Task with depend(depobj:) on pointer */
    if (0) {
        int depobj_target = 0;
        int *depobj_ptr = &depobj_target;
        #pragma omp task depend(depobj: *depobj_ptr)
        {
            *depobj_ptr = 100;
        }
    }
    
    return 0;
}
