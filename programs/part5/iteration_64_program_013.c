/* test-omp-depend-coverage.c
 * 
 * Comprehensive test to trigger pretty-printing of all OMP_CLAUSE_DEPEND_*
 * enumeration values in GCC's tree-pretty-print.cc (lines 824-846).
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
    
    /* 1. OMP_CLAUSE_DEPEND_IN: Simple in dependency */
    if (0) {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
    
    /* 2. OMP_CLAUSE_DEPEND_OUT: Simple out dependency */
    if (0) {
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
    }
    
    /* 3. OMP_CLAUSE_DEPEND_INOUT: Simple inout dependency */
    if (0) {
        #pragma omp task depend(inout: z)
        {
            z++;
        }
    }
    
    /* 4. OMP_CLAUSE_DEPEND_INOUTSET: Set-based inout dependency */
    if (0) {
        #pragma omp task depend(inoutset: arr[0])
        {
            arr[0] = 10;
        }
    }
    
    /* 5. OMP_CLAUSE_DEPEND_MUTEXINOUTSET: Mutex set-based dependency */
    if (0) {
        #pragma omp task depend(mutexinoutset: arr[1])
        {
            arr[1] = 20;
        }
    }
    
    /* 6. OMP_CLAUSE_DEPEND_DEPOBJ: Dependency object */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 30;
        }
    }
    
    /* 7. Multiple dependencies in single construct - triggers iteration through all */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[2]) depend(mutexinoutset: arr[3]) \
                         depend(depobj: depobj_var)
        {
            x = y + z;
            arr[2] = arr[3] + depobj_var;
        }
    }
    
    /* 8. Complex dependency expressions */
    if (0) {
        int *dynamic_ptr = malloc(sizeof(int));
        #pragma omp task depend(in: *ptr) depend(out: *dynamic_ptr) \
                         depend(in: arr[4:2])  /* array section */
        {
            *dynamic_ptr = *ptr + arr[4];
        }
        free(dynamic_ptr);
    }
    
    /* 9. Target region with dependencies */
    if (0) {
        #pragma omp target depend(in: x) depend(out: arr[5]) map(tofrom: arr[5])
        {
            arr[5] = x * 2;
        }
    }
    
    /* 10. Nested tasks in parallel region for complex dependency graph */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(out: x)
                { x = 1; }
                
                #pragma omp task depend(in: x) depend(out: y)
                { y = x + 1; }
                
                #pragma omp task depend(in: y) depend(inoutset: z)
                { z = y + 1; }
                
                #pragma omp task depend(mutexinoutset: arr[6]) depend(in: z)
                { arr[6] = z + 1; }
            }
        }
    }
    
    /* 11. Combined construct with dependency */
    if (0) {
        #pragma omp target parallel for depend(inout: x)
        for (int i = 0; i < 10; i++) {
            arr[i] = x + i;
        }
    }
    
    /* 12. Taskwait with depend clause (different syntax) */
    if (0) {
        #pragma omp task depend(inout: x)
        { x = 100; }
        
        #pragma omp taskwait depend(inoutset: x)
    }
    
    /* Ensure all tasks complete if actually executed */
    #pragma omp taskwait
    
    return 0;
}
