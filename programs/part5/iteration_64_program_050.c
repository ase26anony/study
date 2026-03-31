/* test-omp-depend-coverage.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int len = 10;
    
    /* Block 1: Basic depend clause types in task construct */
    if (0) {
        #pragma omp task depend(in: x)         /* OMP_CLAUSE_DEPEND_IN */
        { x = 1; }
        
        #pragma omp task depend(out: y)        /* OMP_CLAUSE_DEPEND_OUT */
        { y = 2; }
        
        #pragma omp task depend(inout: z)      /* OMP_CLAUSE_DEPEND_INOUT */
        { z = x + y; }
        
        #pragma omp task depend(inoutset: x)   /* OMP_CLAUSE_DEPEND_INOUTSET */
        { x = x * 2; }
        
        #pragma omp task depend(mutexinoutset: y) /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        { y = y / 2; }
        
        #pragma omp task depend(depobj: ptr)   /* OMP_CLAUSE_DEPEND_DEPOBJ */
        { *ptr = 42; }
    }
    
    /* Block 2: Multiple depend clauses on single construct (triggers iteration through list) */
    if (0) {
        /* Multiple clauses ensure pretty-printer iterates through all,
           potentially reaching internal list termination */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[0]) depend(mutexinoutset: arr[1])
        {
            x = y + z;
            arr[0] = arr[1] * 2;
        }
    }
    
    /* Block 3: Depend clauses in target construct */
    if (0) {
        #pragma omp target depend(in: arr[0]) depend(out: arr[1]) map(tofrom: arr[0:2])
        {
            arr[1] = arr[0] + 1;
        }
    }
    
    /* Block 4: Complex dependency expressions */
    if (0) {
        int *dyn_arr = malloc(100 * sizeof(int));
        
        /* Array element with index expression */
        #pragma omp task depend(in: arr[x]) depend(out: arr[y+1])
        { arr[y+1] = arr[x] * 3; }
        
        /* Pointer dereference */
        #pragma omp task depend(inout: *ptr)
        { *ptr += 1; }
        
        /* Iterator modifier (C/C++ syntax for array section) */
        #pragma omp task depend(in: arr[0:len])
        { arr[0] = len; }
        
        free(dyn_arr);
    }
    
    /* Block 5: Nested constructs for complex printing paths */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(inout: x)
                { x++; }
                
                #pragma omp task depend(in: x) depend(out: y)
                { y = x * 2; }
                
                #pragma omp task depend(inoutset: z) depend(mutexinoutset: arr[2])
                {
                    z = x + y;
                    arr[2] = z;
                }
            }
        }
    }
    
    /* Block 6: Combined construct with depend clause */
    if (0) {
        #pragma omp target parallel for depend(out: arr[0]) map(tofrom: arr[0:10])
        for (int i = 0; i < 10; i++) {
            arr[i] = i * i;
        }
    }
    
    /* Block 7: depobj with different variable types */
    if (0) {
        void *depobj_var;
        #pragma omp depobj(depobj_var) depend(inout: x)
        #pragma omp task depend(depobj: depobj_var)
        { x = 100; }
        #pragma omp depobj(depobj_var) destroy
    }
    
    return 0;
}
