/* test-omp-depend-coverage.c
 * 
 * This test program is designed to trigger the pretty-printing logic
 * for all OMP_CLAUSE_DEPEND_* enumeration values in GCC's tree-pretty-print.cc.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * Additional flags: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* 1. OMP_CLAUSE_DEPEND_IN - simple in dependency */
    if (0) {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
    
    /* 2. OMP_CLAUSE_DEPEND_OUT - simple out dependency */
    if (0) {
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
    }
    
    /* 3. OMP_CLAUSE_DEPEND_INOUT - inout dependency */
    if (0) {
        #pragma omp task depend(inout: z)
        {
            z++;
        }
    }
    
    /* 4. OMP_CLAUSE_DEPEND_INOUTSET - set-based inout dependency */
    if (0) {
        #pragma omp task depend(inoutset: arr[0])
        {
            arr[0] = 10;
        }
    }
    
    /* 5. OMP_CLAUSE_DEPEND_MUTEXINOUTSET - mutex set-based dependency */
    if (0) {
        #pragma omp task depend(mutexinoutset: arr[1])
        {
            arr[1] = 20;
        }
    }
    
    /* 6. OMP_CLAUSE_DEPEND_DEPOBJ - dependency object */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 30;
        }
    }
    
    /* 7. Complex expressions and multiple dependencies in single clause */
    if (0) {
        #pragma omp task depend(in: arr[2], *ptr) depend(out: arr[3])
        {
            arr[3] = arr[2] + *ptr;
        }
    }
    
    /* 8. Target region with dependencies */
    if (0) {
        #pragma omp target depend(in: x) depend(out: y) map(tofrom: x, y)
        {
            y = x * 2;
        }
    }
    
    /* 9. Combined construct with dependencies */
    if (0) {
        #pragma omp target parallel for depend(inout: z)
        for (int i = 0; i < 10; i++) {
            arr[i] = i;
        }
    }
    
    /* 10. Nested tasks with multiple dependency types to potentially trigger LAST iteration */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Multiple depend clauses on single construct */
                #pragma omp task depend(in: x) depend(out: y) \
                                 depend(inout: z) depend(mutexinoutset: arr[4]) \
                                 depend(inoutset: arr[5]) depend(depobj: depobj_var)
                {
                    y = x + z;
                    arr[4] = arr[5] + depobj_var;
                }
            }
        }
    }
    
    /* 11. Iterator modifier (C/C++ specific) */
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
    
    /* 12. Taskwait with depend clause */
    if (0) {
        #pragma omp task depend(out: x)
        { x = 1; }
        
        #pragma omp task depend(in: x)
        { y = x + 1; }
        
        #pragma omp taskwait depend(inoutset: x)
    }
    
    /* Ensure all tasks complete if executed */
    #pragma omp taskwait
    
    return 0;
}
