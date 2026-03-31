/* test-depend-clauses.c
 * 
 * This test program is designed to trigger GCC's tree pretty-printer
 * for all OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting
 * the uncovered block in tree-pretty-print.cc lines 824-846.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test-depend-clauses.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
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
        { arr[0] = 10; }
        
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: arr[1])
        { arr[1] = 20; }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y) depend(out: z)
        { z = x + y; }
    }
    
    /* Block 2: depend(depobj) clause - OMP_CLAUSE_DEPEND_DEPOBJ */
    if (0) {
        /* Create a dependency object */
        #pragma omp depobj(depobj_var) depend(inout: x)
        
        /* Use it in a task */
        #pragma omp task depend(depobj: depobj_var)
        { x = x * 2; }
        
        /* Update the dependency object */
        #pragma omp depobj(depobj_var) update(out)
    }
    
    /* Block 3: Target regions with depend clauses */
    if (0) {
        /* OMP_CLAUSE_DEPEND_IN in target */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x = 100; }
        
        /* OMP_CLAUSE_DEPEND_OUT in target */
        #pragma omp target depend(out: arr[2]) map(tofrom: arr[2])
        { arr[2] = 200; }
        
        /* Combined construct with depend */
        #pragma omp target parallel for depend(inout: arr[3]) map(tofrom: arr[3:1])
        for (int i = 0; i < 1; i++) {
            arr[3] = 300;
        }
    }
    
    /* Block 4: Complex dependency expressions */
    if (0) {
        /* Pointer dereference */
        #pragma omp task depend(out: *ptr)
        { *ptr = 42; }
        
        /* Array section with iterator (C/C++ syntax) */
        #pragma omp task depend(in: arr[0:5])
        {
            int sum = 0;
            for (int i = 0; i < 5; i++) sum += arr[i];
        }
        
        /* Multiple array elements */
        #pragma omp task depend(in: arr[0], arr[5]) depend(out: arr[6])
        { arr[6] = arr[0] + arr[5]; }
    }
    
    /* Block 5: Nested constructs for complex printing contexts */
    if (0) {
        #pragma omp parallel
        {
            /* Nested task with depend inside parallel region */
            #pragma omp task depend(inoutset: y)
            { y = y + 1; }
            
            #pragma omp task depend(mutexinoutset: z)
            { z = z * 2; }
            
            /* Taskwait with depend */
            #pragma omp task depend(in: x)
            { x = x - 1; }
            
            #pragma omp taskwait
        }
    }
    
    /* Block 6: Multiple depend clauses on single construct 
     * This helps trigger iteration through clause list including OMP_CLAUSE_DEPEND_LAST */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[7]) depend(mutexinoutset: arr[8])
        {
            y = x;
            z = y + z;
            arr[7] = z;
            arr[8] = arr[7] * 2;
        }
    }
    
    /* Block 7: Sections with depend clauses */
    if (0) {
        #pragma omp parallel sections depend(in: x)
        {
            #pragma omp section
            { y = 1; }
            
            #pragma omp section
            { z = 2; }
        }
    }
    
    /* Ensure variables are used to avoid warnings */
    (void)x; (void)y; (void)z; (void)arr; (void)ptr; (void)depobj_var;
    
    return 0;
}
