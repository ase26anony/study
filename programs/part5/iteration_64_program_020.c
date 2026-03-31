/* test-omp-depend-coverage.c */
/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test.c */

#include <stdlib.h>

int main(void) {
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
        
        /* OMP_CLAUSE_DEPEND_INOUTSET */
        #pragma omp task depend(inoutset: arr[0])
        { arr[0] = 5; }
        
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: arr[1])
        { arr[1] = 6; }
        
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: ptr)
        { *ptr = 7; }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y) depend(out: z)
        { z = x + y; }
    }
    
    if (0) {
        /* Block 2: Target regions with depend clauses */
        /* OMP_CLAUSE_DEPEND_IN in target */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x *= 2; }
        
        /* OMP_CLAUSE_DEPEND_OUT in target */
        #pragma omp target depend(out: arr[2]) map(tofrom: arr[2])
        { arr[2] = 8; }
        
        /* Combined construct */
        #pragma omp target parallel for depend(inout: arr[3]) map(tofrom: arr[3:1])
        for (int i = 0; i < 10; i++) {
            arr[3] += i;
        }
    }
    
    if (0) {
        /* Block 3: Nested parallel region with complex dependencies */
        #pragma omp parallel
        {
            /* OMP_CLAUSE_DEPEND_INOUTSET in nested task */
            #pragma omp task depend(inoutset: y)
            { y++; }
            
            /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET in nested task */
            #pragma omp task depend(mutexinoutset: z)
            { z--; }
            
            /* Iterator modifier (C/C++ specific) */
            #pragma omp task depend(in: arr[0:len])
            { arr[0] = arr[len-1]; }
        }
    }
    
    if (0) {
        /* Block 4: Multiple depend clauses to trigger OMP_CLAUSE_DEPEND_LAST iteration */
        /* This creates a chain of dependencies for the pretty-printer to traverse */
        #pragma omp task depend(in: x) depend(out: y) \
                         depend(inout: z) depend(inoutset: arr[0]) \
                         depend(mutexinoutset: arr[1]) depend(depobj: ptr)
        {
            /* Complex expression to ensure full parsing */
            arr[2] = x + y + z + arr[0] + arr[1] + *ptr;
        }
    }
    
    if (0) {
        /* Block 5: Depend clause with pointer dereference */
        int **pptr = &ptr;
        #pragma omp task depend(in: **pptr)
        { x = **pptr + 1; }
        
        /* Array section with complex subscript */
        #pragma omp task depend(out: arr[x:y])
        { arr[x] = y; }
    }
    
    /* Ensure variables are used to prevent optimization */
    return x + y + z + arr[0] + *ptr;
}
