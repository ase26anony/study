/* test_omp_depend_coverage.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test_omp_depend_coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Block 1: Exhaustive depend clause enumeration in tasks */
    if (0) {
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
        
        /* Multiple clauses to potentially trigger OMP_CLAUSE_DEPEND_LAST iteration */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z)
        { x = y + z; }
    }
    
    /* Block 2: Target regions with depend clauses */
    if (0) {
        /* OMP_CLAUSE_DEPEND_IN with array element */
        #pragma omp target depend(in: arr[2])
        { arr[2] = 100; }
        
        /* OMP_CLAUSE_DEPEND_OUT with pointer dereference */
        #pragma omp target depend(out: *ptr)
        { *ptr = 200; }
        
        /* Combined depend types */
        #pragma omp target depend(in: arr[0], arr[1]) depend(out: arr[3])
        { arr[3] = arr[0] + arr[1]; }
    }
    
    /* Block 3: Parallel region with nested tasks for complex dependency graphs */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Chain of dependencies */
                #pragma omp task depend(out: x)
                { x = 1; }
                
                #pragma omp task depend(in: x) depend(out: y)
                { y = x * 2; }
                
                #pragma omp task depend(in: y) depend(mutexinoutset: z)
                { z = y + 1; }
                
                #pragma omp task depend(inoutset: arr[4]) depend(in: z)
                { arr[4] = z * 10; }
            }
        }
    }
    
    /* Block 4: Combined construct with depend clause */
    if (0) {
        #pragma omp target parallel for depend(inout: arr[5])
        for (int i = 0; i < 10; i++) {
            arr[5] += i;
        }
    }
    
    /* Block 5: Iterator modifier (C/C++ specific) */
    if (0) {
        int len = 5;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i * 2;
            }
        }
    }
    
    /* Block 6: Complex expression in depend clause */
    if (0) {
        int *ptr2 = arr + 2;
        #pragma omp task depend(out: ptr2[0]) depend(in: ptr2[1])
        {
            ptr2[0] = ptr2[1] * 3;
        }
    }
    
    /* Block 7: Multiple depobj clauses */
    if (0) {
        int dep1 = 0, dep2 = 0;
        #pragma omp task depend(depobj: dep1) depend(depobj: dep2)
        {
            dep1 = dep2 = 42;
        }
    }
    
    return 0;
}
