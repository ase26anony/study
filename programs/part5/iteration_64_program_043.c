/* test-omp-depend-coverage.c
 * 
 * This test is designed to trigger GCC's tree pretty-printer for all
 * OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting the
 * uncovered block in tree-pretty-print.cc lines 824-846.
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
    
    /* 1. OMP_CLAUSE_DEPEND_IN: Simple in dependency */
    if (0) {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
    
    /* 2. OMP_CLAUSE_DEPEND_OUT: Simple out dependency */
    if (0) {
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
    }
    
    /* 3. OMP_CLAUSE_DEPEND_INOUT: Inout dependency */
    if (0) {
        #pragma omp task depend(inout: z)
        {
            z++;
        }
    }
    
    /* 4. OMP_CLAUSE_DEPEND_INOUTSET: Inoutset dependency */
    if (0) {
        #pragma omp task depend(inoutset: arr[0])
        {
            arr[0] = 10;
        }
    }
    
    /* 5. OMP_CLAUSE_DEPEND_MUTEXINOUTSET: Mutexinoutset dependency */
    if (0) {
        #pragma omp task depend(mutexinoutset: arr[1])
        {
            arr[1] = 20;
        }
    }
    
    /* 6. OMP_CLAUSE_DEPEND_DEPOBJ: Depobj dependency */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 30;
        }
    }
    
    /* 7. Multiple dependencies in single clause to test pretty-printing */
    if (0) {
        #pragma omp task depend(in: x, y) depend(out: z)
        {
            z = x + y;
        }
    }
    
    /* 8. Target region with dependencies */
    if (0) {
        #pragma omp target depend(in: arr[2]) depend(out: arr[3])
        {
            arr[3] = arr[2] * 2;
        }
    }
    
    /* 9. Combined construct with dependencies */
    if (0) {
        #pragma omp target parallel for depend(inout: arr[4])
        for (int i = 0; i < 10; i++) {
            arr[i] = i;
        }
    }
    
    /* 10. Nested tasks with complex dependencies to trigger OMP_CLAUSE_DEPEND_LAST
     *     The pretty-printer iterates through multiple clauses, potentially reaching
     *     the end of the clause list where LAST might be encountered internally.
     */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp task depend(in: x) depend(out: y)
            {
                y = x;
            }
            
            #pragma omp task depend(in: y) depend(out: z) depend(inout: arr[5])
            {
                z = y + arr[5];
            }
        }
    }
    
    /* 11. Pointer dereference in dependency */
    if (0) {
        #pragma omp task depend(out: *ptr)
        {
            *ptr = 100;
        }
    }
    
    /* 12. Array section with iterator (C/C++ syntax) */
    if (0) {
        int len = 5;
        #pragma omp task depend(in: arr[0:len])
        {
            int sum = 0;
            for (int i = 0; i < len; i++) {
                sum += arr[i];
            }
        }
    }
    
    /* 13. Taskwait with depend clause */
    if (0) {
        #pragma omp task depend(out: x)
        { x = 1; }
        
        #pragma omp task depend(in: x)
        { y = x; }
        
        #pragma omp taskwait depend(inoutset: x)
    }
    
    /* 14. Taskloop with dependencies */
    if (0) {
        #pragma omp taskloop depend(inout: arr[6]) grainsize(1)
        for (int i = 0; i < 10; i++) {
            arr[i] += i;
        }
    }
    
    /* 15. Taskgroup with task dependencies inside */
    if (0) {
        #pragma omp taskgroup
        {
            #pragma omp task depend(mutexinoutset: arr[7])
            {
                arr[7] = 77;
            }
            
            #pragma omp task depend(inoutset: arr[8])
            {
                arr[8] = 88;
            }
        }
    }
    
    return 0;
}
