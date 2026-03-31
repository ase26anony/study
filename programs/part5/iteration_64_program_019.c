/* test_depend_clauses.c - Exhaustive test for OMP_CLAUSE_DEPEND_* pretty-printing */
/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test_depend_clauses.c */

#include <stdlib.h>

int main(void) {
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Block 1: Basic depend clause types in task construct */
    if (0) {
        #pragma omp task depend(in: x) default(none)
        { x = 1; }
        
        #pragma omp task depend(out: y) default(none)
        { y = 2; }
        
        #pragma omp task depend(inout: z) default(none)
        { z = x + y; }
    }
    
    /* Block 2: Set-based depend types in nested tasks */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(mutexinoutset: x) default(none) shared(x)
                { x = x + 1; }
                
                #pragma omp task depend(inoutset: y) default(none) shared(y)
                { y = y * 2; }
                
                #pragma omp task depend(inoutset: x, y) default(none) shared(x, y)
                { x = y; y = x; }
            }
        }
    }
    
    /* Block 3: depobj modifier with pointer */
    if (0) {
        #pragma omp task depend(depobj: depobj_var) default(none)
        { depobj_var = 42; }
        
        #pragma omp task depend(depobj: *ptr) default(none)
        { *ptr = 100; }
    }
    
    /* Block 4: Complex dependency expressions in target region */
    if (0) {
        #pragma omp target depend(in: arr[0]) depend(out: arr[1]) map(tofrom: arr)
        {
            arr[1] = arr[0] + 10;
        }
        
        /* Multiple items in single clause */
        #pragma omp target teams distribute parallel for depend(in: arr[0], arr[2]) depend(out: arr[3], arr[4])
        for (int i = 0; i < 10; i++) {
            arr[i] = i;
        }
    }
    
    /* Block 5: Combined construct with iterator modifier (C/C++ specific) */
    if (0) {
        int len = 10;
        #pragma omp task depend(in: arr[0:len]) default(none) shared(arr)
        {
            for (int i = 0; i < len; i++) {
                arr[i] = arr[i] * 2;
            }
        }
    }
    
    /* Block 6: Multiple depend clauses to trigger OMP_CLAUSE_DEPEND_LAST iteration */
    if (0) {
        /* This creates a clause list where the printer iterates through all */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(mutexinoutset: arr[0]) depend(inoutset: arr[1]) \
                         depend(depobj: depobj_var) default(none)
        {
            x = y + z;
            arr[0] = arr[1];
            depobj_var = x;
        }
    }
    
    /* Block 7: depend clause in taskwait */
    if (0) {
        #pragma omp task depend(inout: x)
        { x = 1; }
        
        #pragma omp taskwait depend(inoutset: x)
    }
    
    /* Block 8: depend clause in taskloop */
    if (0) {
        #pragma omp taskloop depend(in: arr) grainsize(2)
        for (int i = 0; i < 10; i++) {
            arr[i] = i * 2;
        }
    }
    
    /* Block 9: Nested dependency graph for complex printing */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(out: x)  // OMP_CLAUSE_DEPEND_OUT
                { x = 1; }
                
                #pragma omp task depend(in: x) depend(out: y)  // Multiple types
                { y = x + 1; }
                
                #pragma omp task depend(in: y) depend(inout: z)  // IN and INOUT
                { z = y * 2; }
                
                #pragma omp task depend(mutexinoutset: z)  // MUTEXINOUTSET
                { z = z + 1; }
            }
        }
    }
    
    return 0;
}
