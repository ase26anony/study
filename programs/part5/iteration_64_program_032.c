/* test_omp_depend_coverage.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test_omp_depend_coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for all dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* 1. Basic depend(in:) clause */
    if (0) {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
    
    /* 2. Basic depend(out:) clause */
    if (0) {
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
    }
    
    /* 3. Basic depend(inout:) clause */
    if (0) {
        #pragma omp task depend(inout: z)
        {
            z = x + y;
        }
    }
    
    /* 4. depend(depobj:) clause - specific to depobj modifier */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 42;
        }
    }
    
    /* 5. depend(mutexinoutset:) clause */
    if (0) {
        #pragma omp task depend(mutexinoutset: arr[0])
        {
            arr[0] = 100;
        }
    }
    
    /* 6. depend(inoutset:) clause */
    if (0) {
        #pragma omp task depend(inoutset: arr[1])
        {
            arr[1] = 200;
        }
    }
    
    /* 7. Multiple depend clauses on single construct to potentially trigger LAST iteration */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z)
        {
            z = x + y;
        }
    }
    
    /* 8. Complex dependency expressions with array elements */
    if (0) {
        #pragma omp task depend(in: arr[2]) depend(out: arr[3])
        {
            arr[3] = arr[2] * 2;
        }
    }
    
    /* 9. Pointer dereference in depend clause */
    if (0) {
        #pragma omp task depend(inout: *ptr)
        {
            *ptr += 1;
        }
    }
    
    /* 10. depend clause in target region */
    if (0) {
        #pragma omp target depend(in: x) depend(out: arr[4])
        {
            arr[4] = x;
        }
    }
    
    /* 11. depend clause in combined construct */
    if (0) {
        #pragma omp target parallel for depend(out: arr[5])
        for (int i = 0; i < 10; i++) {
            arr[5] += i;
        }
    }
    
    /* 12. Nested tasks with dependencies inside parallel region */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(out: arr[6])
                { arr[6] = 1; }
                
                #pragma omp task depend(in: arr[6]) depend(out: arr[7])
                { arr[7] = arr[6] * 2; }
                
                #pragma omp task depend(in: arr[7]) depend(mutexinoutset: arr[8])
                { arr[8] = arr[7] + 1; }
            }
        }
    }
    
    /* 13. Iterator modifier (C/C++ specific) with depend clause */
    if (0) {
        int len = 5;
        #pragma omp task depend(in: arr[2:len])
        {
            for (int i = 2; i < 2 + len; i++) {
                arr[i] = i;
            }
        }
    }
    
    /* 14. Mixed dependency types including depobj */
    if (0) {
        #pragma omp task depend(depobj: depobj_var) depend(in: x) depend(out: y) \
                         depend(inout: z) depend(mutexinoutset: arr[0]) \
                         depend(inoutset: arr[1])
        {
            /* Complex operation using all dependencies */
            z = x + y + depobj_var + arr[0] + arr[1];
        }
    }
    
    /* 15. Taskwait with depend clause */
    if (0) {
        #pragma omp task depend(out: x)
        { x = 10; }
        
        #pragma omp taskwait depend(in: x)
        
        #pragma omp task depend(in: x)
        { y = x * 2; }
    }
    
    return 0;
}
