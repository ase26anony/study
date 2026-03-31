/* test-depend-clauses.c
 * 
 * This test program is designed to trigger GCC's tree pretty-printer
 * for all OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting
 * the uncovered block in tree-pretty-print.cc (lines 824-846).
 * 
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test-depend-clauses.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* 1. OMP_CLAUSE_DEPEND_IN: Simple task with 'in' dependency */
    if (0) {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
    
    /* 2. OMP_CLAUSE_DEPEND_OUT: Task with 'out' dependency on array element */
    if (0) {
        #pragma omp task depend(out: arr[0])
        {
            arr[0] = 42;
        }
    }
    
    /* 3. OMP_CLAUSE_DEPEND_INOUT: Task with 'inout' dependency */
    if (0) {
        #pragma omp task depend(inout: y)
        {
            y += 5;
        }
    }
    
    /* 4. OMP_CLAUSE_DEPEND_INOUTSET: Task with 'inoutset' dependency */
    if (0) {
        #pragma omp task depend(inoutset: z)
        {
            z = 100;
        }
    }
    
    /* 5. OMP_CLAUSE_DEPEND_MUTEXINOUTSET: Task with 'mutexinoutset' dependency */
    if (0) {
        #pragma omp task depend(mutexinoutset: y)
        {
            y *= 2;
        }
    }
    
    /* 6. OMP_CLAUSE_DEPEND_DEPOBJ: Task with 'depobj' dependency */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 99;
        }
    }
    
    /* 7. Multiple dependencies in single clause to test pretty-printing */
    if (0) {
        int a = 0, b = 0, c = 0;
        #pragma omp task depend(in: a, b) depend(out: c)
        {
            c = a + b;
        }
    }
    
    /* 8. Pointer dereference in dependency */
    if (0) {
        #pragma omp task depend(out: *ptr)
        {
            *ptr = 77;
        }
    }
    
    /* 9. Target region with dependencies (tests different construct path) */
    if (0) {
        #pragma omp target depend(in: arr[0]) depend(out: arr[1])
        {
            arr[1] = arr[0] + 1;
        }
    }
    
    /* 10. Combined construct: target parallel for with dependency */
    if (0) {
        #pragma omp target parallel for depend(inout: x)
        for (int i = 0; i < 10; i++) {
            arr[i] = i;
        }
    }
    
    /* 11. Nested tasks inside parallel region for complex dependency graph */
    if (0) {
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
                
                #pragma omp task depend(inoutset: z)
                { z += 10; }
            }
        }
    }
    
    /* 12. Iterator modifier (C/C++ specific) for array section */
    if (0) {
        int len = 10;
        #pragma omp task depend(in: arr[0:len])
        {
            int sum = 0;
            for (int i = 0; i < len; i++) {
                sum += arr[i];
            }
        }
    }
    
    /* 13. Multiple depend clauses to potentially trigger OMP_CLAUSE_DEPEND_LAST
     *     during iteration through clause list */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(mutexinoutset: arr[0]) depend(inoutset: arr[1])
        {
            /* Complex operation with many dependencies */
            y = x + z;
            arr[0] = y;
            arr[1] = z;
        }
    }
    
    /* 14. Taskwait with depend clause */
    if (0) {
        #pragma omp task depend(out: x)
        { x = 10; }
        
        #pragma omp taskwait depend(in: x)
    }
    
    /* 15. Taskloop with dependencies */
    if (0) {
        #pragma omp taskloop depend(in: x) depend(out: arr) grainsize(2)
        for (int i = 0; i < 10; i++) {
            arr[i] = x + i;
        }
    }
    
    return 0;
}
