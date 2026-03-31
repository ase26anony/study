/* test-omp-depend-coverage.c
 * 
 * This test program is designed to trigger GCC's internal pretty-printing
 * logic for all OpenMP `depend` clause variants, specifically targeting
 * the uncovered block in tree-pretty-print.cc lines 824-846.
 *
 * Compile with tree dumping enabled to see the pretty-printed output:
 *   gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test.c
 *   gcc -O2 -fopenmp -fdump-tree-optimized test.c
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x, y, z;
    int arr[10];
    int *ptr = &x;
    int depobj_var;
    
    /* 1. Basic depend(in:) clause in a task */
    if (0) {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
    
    /* 2. Basic depend(out:) clause in a task */
    if (0) {
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
    }
    
    /* 3. Basic depend(inout:) clause in a task */
    if (0) {
        #pragma omp task depend(inout: z)
        {
            z = z + 1;
        }
    }
    
    /* 4. depend(depobj:) clause - specific to dependency objects */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 42;
        }
    }
    
    /* 5. depend(mutexinoutset:) clause for set-based synchronization */
    if (0) {
        #pragma omp task depend(mutexinoutset: x)
        {
            x = x * 2;
        }
    }
    
    /* 6. depend(inoutset:) clause for set-based dependencies */
    if (0) {
        #pragma omp task depend(inoutset: y)
        {
            y = y + 10;
        }
    }
    
    /* 7. Multiple depend clauses on a single construct 
     * This may help trigger OMP_CLAUSE_DEPEND_LAST iteration */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z)
        {
            z = x + y;
        }
    }
    
    /* 8. Complex dependency expressions: array element */
    if (0) {
        #pragma omp task depend(out: arr[0])
        {
            arr[0] = 100;
        }
    }
    
    /* 9. Complex dependency expressions: pointer dereference */
    if (0) {
        #pragma omp task depend(in: *ptr)
        {
            int temp = *ptr;
        }
    }
    
    /* 10. Iterator modifier (C/C++ specific) */
    if (0) {
        #pragma omp task depend(in: arr[0:5])
        {
            int sum = 0;
            for (int i = 0; i < 5; i++) {
                sum += arr[i];
            }
        }
    }
    
    /* 11. Target region with depend clause */
    if (0) {
        #pragma omp target depend(out: arr) map(tofrom: arr)
        {
            arr[0] = 999;
        }
    }
    
    /* 12. Combined construct with depend clause */
    if (0) {
        #pragma omp target parallel for depend(in: x)
        for (int i = 0; i < 10; i++) {
            arr[i] = i;
        }
    }
    
    /* 13. Nested tasks with dependencies inside parallel region */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(out: x)
                { x = 1; }
                
                #pragma omp task depend(in: x) depend(out: y)
                { y = x + 1; }
                
                #pragma omp task depend(in: y) depend(mutexinoutset: z)
                { z = y * 2; }
                
                #pragma omp task depend(inoutset: z)
                { z = z + 5; }
            }
        }
    }
    
    /* 14. Task with depend(depobj:) and other depend types combined */
    if (0) {
        #pragma omp task depend(depobj: depobj_var) depend(in: x) depend(out: y)
        {
            y = x + depobj_var;
        }
    }
    
    /* 15. Multiple tasks forming a dependency chain with various types */
    if (0) {
        #pragma omp task depend(out: x)
        { x = 10; }
        
        #pragma omp task depend(in: x) depend(out: y)
        { y = x * 2; }
        
        #pragma omp task depend(inout: y) depend(out: z)
        { z = y + 5; y = 0; }
        
        #pragma omp task depend(mutexinoutset: z) depend(inoutset: x)
        { x = z - 5; }
        
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = x + y + z; }
    }
    
    /* Ensure all tasks complete if the program were actually run */
    #pragma omp taskwait
    
    return 0;
}
