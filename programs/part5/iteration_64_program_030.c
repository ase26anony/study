/* test_depend_clauses.c - Exhaustive test for OMP_CLAUSE_DEPEND_* pretty-printing */
/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test_depend_clauses.c */

#include <stdlib.h>

int main(void) {
    /* Declare variables for all dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Block 1: Basic depend clause types in task construct */
    if (0) {
        #pragma omp task depend(in: x) firstprivate(x)
        { x = 1; }
        
        #pragma omp task depend(out: y) 
        { y = 2; }
        
        #pragma omp task depend(inout: z)
        { z = x + y; }
        
        #pragma omp taskwait
    }
    
    /* Block 2: Set-based dependencies in nested tasks */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(mutexinoutset: arr[0]) 
                { arr[0] = 1; }
                
                #pragma omp task depend(inoutset: arr[1])
                { arr[1] = arr[0] + 1; }
                
                #pragma omp task depend(inoutset: arr[0], arr[1])
                { arr[2] = arr[0] + arr[1]; }
            }
        }
    }
    
    /* Block 3: depobj modifier (OpenMP 5.0+) */
    if (0) {
        #pragma omp depobj(depobj_var) update(inout)
        
        #pragma omp task depend(depobj: depobj_var)
        { x = 42; }
        
        #pragma omp task depend(depobj: depobj_var)
        { y = 24; }
    }
    
    /* Block 4: Multiple depend clauses on single construct (triggers iteration) */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(mutexinoutset: arr[0]) depend(inoutset: arr[1])
        {
            y = x + z;
            arr[1] = arr[0] + y;
        }
    }
    
    /* Block 5: Target regions with dependencies */
    if (0) {
        #pragma omp target map(tofrom: x) depend(in: x)
        { x = x * 2; }
        
        #pragma omp target map(tofrom: arr) depend(out: arr[0:5])
        {
            for (int i = 0; i < 5; i++)
                arr[i] = i;
        }
    }
    
    /* Block 6: Combined construct with iterator modifier */
    if (0) {
        int len = 10;
        #pragma omp target parallel for depend(in: arr[0:len]) map(tofrom: arr)
        for (int i = 0; i < len; i++) {
            arr[i] = arr[i] * 2;
        }
    }
    
    /* Block 7: Complex pointer expressions */
    if (0) {
        int **pptr = &ptr;
        #pragma omp task depend(in: *ptr) depend(out: **pptr)
        {
            **pptr = 100;
        }
    }
    
    /* Block 8: Task with depend clause containing multiple items */
    if (0) {
        int a = 1, b = 2, c = 3, d = 4;
        #pragma omp task depend(in: a, b) depend(out: c, d)
        {
            c = a + b;
            d = a - b;
        }
    }
    
    /* Block 9: Nested tasks creating dependency chain */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(out: x)      /* T1 */
                { x = 1; }
                
                #pragma omp task depend(in: x)       /* T2 */
                { y = x + 1; }
                
                #pragma omp task depend(in: x)       /* T3 */
                { z = x * 2; }
                
                #pragma omp task depend(in: y, z)    /* T4 */
                { arr[0] = y + z; }
            }
        }
    }
    
    return 0;
}
