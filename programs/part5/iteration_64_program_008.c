/* test-omp-depend-coverage.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer for
 * all OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting
 * the uncovered block in tree-pretty-print.cc (lines 824-846).
 * 
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
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
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 7; }
    }
    
    if (0) {
        /* Block 2: Multiple depend clauses in single construct 
           This helps trigger iteration through all clause types,
           potentially reaching OMP_CLAUSE_DEPEND_LAST */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[0]) depend(mutexinoutset: arr[1]) \
                         depend(depobj: depobj_var)
        {
            x = y + z;
            arr[0] = arr[1] + depobj_var;
        }
    }
    
    if (0) {
        /* Block 3: Complex dependency expressions */
        int *dynamic_ptr = malloc(sizeof(int));
        
        /* Array element with index */
        #pragma omp task depend(in: arr[x])
        { arr[x] = 10; }
        
        /* Pointer dereference */
        #pragma omp task depend(out: *dynamic_ptr)
        { *dynamic_ptr = 20; }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y, z)
        { arr[2] = x + y + z; }
        
        free(dynamic_ptr);
    }
    
    if (0) {
        /* Block 4: Depend clauses in target regions */
        #pragma omp target depend(in: x) depend(out: arr[0])
        { arr[0] = x * 2; }
        
        /* Combined construct */
        #pragma omp target parallel for depend(inout: y)
        for (int i = 0; i < 10; i++) {
            arr[i] = y + i;
        }
    }
    
    if (0) {
        /* Block 5: Nested tasks with dependencies */
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Producer task */
                #pragma omp task depend(out: x)
                { x = 100; }
                
                /* Consumer task with multiple dependency types */
                #pragma omp task depend(in: x) depend(mutexinoutset: y)
                { y = x + 50; }
                
                /* Another consumer with different type */
                #pragma omp task depend(in: x) depend(inoutset: z)
                { z = x * 2; }
            }
        }
    }
    
    if (0) {
        /* Block 6: Iterator modifier (C/C++ specific) */
        int len = 10;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i;
            }
        }
    }
    
    /* Ensure all tasks complete if somehow executed */
    #pragma omp taskwait
    
    return 0;
}
