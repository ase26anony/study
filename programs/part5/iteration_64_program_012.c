/* test-omp-depend-coverage.c
 * 
 * This test is designed to trigger GCC's internal pretty-printing logic
 * for all OMP_CLAUSE_DEPEND_* enumeration values in tree-pretty-print.cc.
 * Compile with -fopenmp -fdump-tree-omplower -fdump-tree-gimple
 * to see the pretty-printed output in .omplower and .gimple dump files.
 */

#include <stdlib.h>

int main(void) {
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int len = 10;
    
    /* Block 1: Basic depend clause types in tasks */
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
        { arr[0] = 5; }
        
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: arr[1])
        { arr[1] = 6; }
        
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: ptr)
        { *ptr = 7; }
        
        #pragma omp taskwait
    }
    
    /* Block 2: Multiple depend clauses on single construct 
       (may help trigger OMP_CLAUSE_DEPEND_LAST iteration) */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z)
        {
            z = x;
            y = z + 1;
        }
        
        #pragma omp taskwait
    }
    
    /* Block 3: Depend clauses in target regions */
    if (0) {
        /* OMP_CLAUSE_DEPEND_IN with array element */
        #pragma omp target depend(in: arr[2])
        { arr[2] = 8; }
        
        /* OMP_CLAUSE_DEPEND_OUT with pointer dereference */
        #pragma omp target depend(out: *ptr)
        { *ptr = 9; }
        
        /* Combined depend types */
        #pragma omp target depend(inoutset: arr[3]) depend(mutexinoutset: arr[4])
        {
            arr[3] = 10;
            arr[4] = 11;
        }
    }
    
    /* Block 4: Depend clauses in parallel regions with tasks */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Nested tasks with various depend types */
                #pragma omp task depend(in: x)
                { arr[5] = x; }
                
                #pragma omp task depend(out: arr[6])
                { arr[6] = 12; }
                
                #pragma omp task depend(inout: y) depend(depobj: ptr)
                {
                    y = 13;
                    *ptr = y;
                }
                
                #pragma omp task depend(mutexinoutset: arr[7])
                { arr[7] = 14; }
                
                #pragma omp task depend(inoutset: arr[8])
                { arr[8] = 15; }
            }
        }
    }
    
    /* Block 5: Iterator modifier (C/C++ specific) */
    if (0) {
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i;
            }
        }
        
        #pragma omp taskwait
    }
    
    /* Block 6: Combined construct with depend clause */
    if (0) {
        #pragma omp target parallel for depend(inout: arr[9])
        for (int i = 0; i < 10; i++) {
            arr[9] += i;
        }
    }
    
    return 0;
}
