/* test-omp-depend-coverage.c
 * 
 * This test is designed to trigger the pretty-printing logic for all
 * OMP_CLAUSE_DEPEND_* enumeration values in GCC's tree-pretty-print.cc.
 * Compile with flags like: -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple
 * The program doesn't need to execute correctly; it only needs to be compiled
 * to generate the tree dumps where the pretty-printer will be invoked.
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables used in depend clauses */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Block 1: Basic depend kinds in simple task */
    if (0) {
        #pragma omp task depend(in: x)        /* OMP_CLAUSE_DEPEND_IN */
        { x = 1; }
        
        #pragma omp task depend(out: y)       /* OMP_CLAUSE_DEPEND_OUT */
        { y = 2; }
        
        #pragma omp task depend(inout: z)     /* OMP_CLAUSE_DEPEND_INOUT */
        { z = x + y; }
    }
    
    /* Block 2: Set-based depend kinds in nested tasks */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(mutexinoutset: arr[0])  /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
                { arr[0] = 1; }
                
                #pragma omp task depend(inoutset: arr[1])       /* OMP_CLAUSE_DEPEND_INOUTSET */
                { arr[1] = arr[0] + 1; }
                
                #pragma omp task depend(inoutset: arr[2])       /* Another INOUTSET */
                { arr[2] = arr[1] * 2; }
            }
        }
    }
    
    /* Block 3: depobj kind with pointer */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)    /* OMP_CLAUSE_DEPEND_DEPOBJ */
        { depobj_var = 42; }
        
        #pragma omp task depend(depobj: *ptr)          /* depobj with dereference */
        { *ptr = 100; }
    }
    
    /* Block 4: Multiple depend clauses on single construct (to reach LAST) */
    if (0) {
        /* Multiple clauses force iteration through all, potentially hitting LAST */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(mutexinoutset: arr[0]) depend(inoutset: arr[1])
        {
            x = y + z;
            arr[0] = arr[1];
        }
    }
    
    /* Block 5: Target regions with depend clauses */
    if (0) {
        #pragma omp target depend(in: x) depend(out: arr[0]) map(tofrom: x, arr[0:1])
        {
            arr[0] = x * 2;
        }
        
        #pragma omp target teams distribute parallel for depend(out: arr[0:10]) \
                         map(tofrom: arr[0:10])
        for (int i = 0; i < 10; i++) {
            arr[i] = i;
        }
    }
    
    /* Block 6: Iterator modifier (C/C++ specific) */
    if (0) {
        int len = 10;
        #pragma omp task depend(in: arr[0:len])  /* iterator syntax */
        {
            int sum = 0;
            for (int i = 0; i < len; i++) sum += arr[i];
        }
    }
    
    /* Block 7: Complex combined construct */
    if (0) {
        #pragma omp target parallel for depend(inout: x) \
                     map(tofrom: x) shared(arr)
        for (int i = 0; i < 10; i++) {
            arr[i] = x + i;
        }
    }
    
    /* Ensure all tasks complete if actually executed */
    #pragma omp taskwait
    
    return 0;
}
