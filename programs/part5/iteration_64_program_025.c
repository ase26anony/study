/* test-omp-depend-coverage.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer for all
 * OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting the
 * uncovered lines in tree-pretty-print.cc (lines 824-846).
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
            y++;
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
            y = 200;
        }
    }
    
    /* 6. OMP_CLAUSE_DEPEND_DEPOBJ: Task with 'depobj' dependency */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 300;
        }
    }
    
    /* 7. Multiple dependencies in single construct - triggers iteration through all types */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[1]) depend(mutexinoutset: arr[2]) \
                         depend(depobj: depobj_var)
        {
            x = y + z;
        }
    }
    
    /* 8. Target region with dependencies - tests different construct context */
    if (0) {
        #pragma omp target depend(in: arr[0]) depend(out: arr[5]) map(tofrom: arr[0:10])
        {
            arr[0] = arr[5] + 1;
        }
    }
    
    /* 9. Parallel region with nested tasks - complex dependency graph */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(out: x)
                { x = 1; }
                
                #pragma omp task depend(in: x) depend(out: y)
                { y = x + 1; }
                
                #pragma omp task depend(inoutset: y) depend(out: z)
                { z = y * 2; }
                
                #pragma omp task depend(mutexinoutset: z) depend(depobj: depobj_var)
                { depobj_var = z; }
            }
        }
    }
    
    /* 10. Combined construct with dependency */
    if (0) {
        #pragma omp target parallel for depend(inout: x)
        for (int i = 0; i < 10; i++) {
            arr[i] = x + i;
        }
    }
    
    /* 11. Iterator modifier (C/C++ specific) for array section */
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
    
    /* 12. Pointer dereference in dependency */
    if (0) {
        #pragma omp task depend(out: *ptr)
        {
            *ptr = 999;
        }
    }
    
    /* 13. Multiple items in single clause */
    if (0) {
        #pragma omp task depend(in: x, y, arr[3]) depend(out: z, arr[4])
        {
            z = x + y + arr[3];
            arr[4] = z;
        }
    }
    
    /* 14. Dependency on taskwait (implicitly creates depend objects) */
    if (0) {
        #pragma omp task depend(out: x)
        { x = 1; }
        
        #pragma omp task depend(in: x)
        { y = x + 1; }
        
        #pragma omp taskwait
    }
    
    /* Force a return to avoid "control reaches end of non-void function" warning */
    return 0;
}
