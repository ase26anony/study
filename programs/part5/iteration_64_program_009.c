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
    
    /* Block 1: Basic depend clause types in task construct */
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
        
        /* Multiple dependencies in single clause */
        #pragma omp task depend(in: x, y) depend(out: z)
        { z = x + y; }
    }
    
    /* Block 2: Array elements and pointer dereferences */
    if (0) {
        /* Array element dependency */
        #pragma omp task depend(in: arr[0])
        { arr[0] = 10; }
        
        /* Pointer dereference dependency */
        #pragma omp task depend(out: *ptr)
        { *ptr = 20; }
        
        /* Mixed dependencies */
        #pragma omp task depend(in: arr[1], *ptr) depend(out: arr[2])
        { arr[2] = arr[1] + *ptr; }
    }
    
    /* Block 3: Set-based dependency types (mutexinoutset, inoutset) */
    if (0) {
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: x)
        { x = x + 1; }
        
        /* OMP_CLAUSE_DEPEND_INOUTSET */
        #pragma omp task depend(inoutset: y)
        { y = y * 2; }
        
        /* Both set types in nested parallel region */
        #pragma omp parallel
        {
            #pragma omp task depend(mutexinoutset: x) depend(inoutset: y)
            { x = y; y = x; }
        }
    }
    
    /* Block 4: depobj dependency type */
    if (0) {
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 100; }
        
        /* depobj with pointer */
        int *depobj_ptr = &depobj_var;
        #pragma omp task depend(depobj: *depobj_ptr)
        { *depobj_ptr = 200; }
    }
    
    /* Block 5: Iterator modifier (C/C++ specific) */
    if (0) {
        int len = 10;
        /* Iterator dependency - should trigger pretty-printing of iterator syntax */
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i;
            }
        }
    }
    
    /* Block 6: Target regions with dependencies */
    if (0) {
        /* Target with in dependency */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x = x * 2; }
        
        /* Target with out dependency */
        #pragma omp target depend(out: arr[0]) map(tofrom: arr[0:5])
        { arr[0] = 42; }
        
        /* Target with multiple dependencies */
        #pragma omp target depend(in: x) depend(out: y) map(tofrom: x, y)
        { y = x; x = 0; }
    }
    
    /* Block 7: Combined constructs with dependencies */
    if (0) {
        /* Combined target parallel for with dependency */
        #pragma omp target parallel for depend(inout: x) map(tofrom: x, arr)
        for (int i = 0; i < 10; i++) {
            arr[i] = x + i;
        }
    }
    
    /* Block 8: Complex nested dependency graph to potentially trigger LAST iteration */
    if (0) {
        /* Multiple depend clauses on single construct - may cause iteration to LAST */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(mutexinoutset: arr[0]) depend(inoutset: arr[1])
        {
            x = y + z;
            arr[0] = arr[1] * 2;
        }
        
        /* Chain of dependencies */
        #pragma omp task depend(out: x)
        { x = 1; }
        
        #pragma omp task depend(in: x) depend(out: y)
        { y = x + 1; }
        
        #pragma omp task depend(in: y) depend(out: z)
        { z = y + 1; }
        
        #pragma omp task depend(in: z) depend(out: arr[0])
        { arr[0] = z; }
    }
    
    /* Block 9: Sections with task dependencies */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp sections
            {
                #pragma omp section
                {
                    #pragma omp task depend(in: x)
                    { y = x + 1; }
                }
                #pragma omp section
                {
                    #pragma omp task depend(out: z)
                    { z = 2; }
                }
            }
        }
    }
    
    return 0;
}
