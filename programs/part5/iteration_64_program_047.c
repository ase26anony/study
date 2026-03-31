/* test-omp-depend-coverage.c
 * 
 * This program is designed to trigger GCC's tree pretty-printer for
 * all OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting
 * the uncovered block in tree-pretty-print.cc (lines 824-846).
 * 
 * Compile with: 
 *   gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * 
 * Additional dump flags for more contexts:
 *   -fdump-tree-original -fdump-tree-optimized -fdump-tree-all
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
        /* Block 1: Basic task with standard depend clauses */
        /* OMP_CLAUSE_DEPEND_IN */
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
        
        /* OMP_CLAUSE_DEPEND_OUT */
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
        
        /* OMP_CLAUSE_DEPEND_INOUT */
        #pragma omp task depend(inout: z)
        {
            z = x + y;
        }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y) depend(out: z)
        {
            z = x + y;
        }
    }
    
    if (0) {
        /* Block 2: Target region with array/pointer dependencies */
        /* Array element dependency */
        #pragma omp target depend(out: arr[0])
        {
            arr[0] = 42;
        }
        
        /* Pointer dereference dependency */
        #pragma omp target depend(inout: *ptr)
        {
            *ptr += 1;
        }
        
        /* Combined target parallel with depend */
        #pragma omp target parallel for depend(in: arr[0:5])
        for (int i = 0; i < 5; i++) {
            arr[i] = i;
        }
    }
    
    if (0) {
        /* Block 3: Set-based dependencies in nested tasks */
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
                #pragma omp task depend(mutexinoutset: y)
                {
                    y = y * 2;
                }
                
                /* OMP_CLAUSE_DEPEND_INOUTSET */
                #pragma omp task depend(inoutset: z)
                {
                    z = z + 1;
                }
                
                /* Multiple set dependencies */
                #pragma omp task depend(mutexinoutset: y, z)
                {
                    y = z;
                    z = y;
                }
            }
        }
    }
    
    if (0) {
        /* Block 4: depobj dependency */
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 100;
        }
        
        /* depobj with pointer */
        int *depobj_ptr = &depobj_var;
        #pragma omp task depend(depobj: *depobj_ptr)
        {
            *depobj_ptr += 50;
        }
    }
    
    if (0) {
        /* Block 5: Complex dependency graph to trigger iteration through all clauses */
        /* This may help reach OMP_CLAUSE_DEPEND_LAST during iteration */
        #pragma omp task depend(in: x) \
                         depend(out: y) \
                         depend(inout: z) \
                         depend(mutexinoutset: arr[0]) \
                         depend(inoutset: arr[1]) \
                         depend(depobj: depobj_var)
        {
            /* Complex operation using all dependencies */
            y = x + z + arr[0] + arr[1] + depobj_var;
        }
    }
    
    if (0) {
        /* Block 6: Iterator modifier for array section (C/C++ syntax) */
        int len = 10;
        #pragma omp task depend(in: arr[0:len])
        {
            int sum = 0;
            for (int i = 0; i < len; i++) {
                sum += arr[i];
            }
            x = sum;
        }
    }
    
    if (0) {
        /* Block 7: Sections with dependencies */
        #pragma omp parallel sections depend(in: x)
        {
            #pragma omp section
            {
                y = x + 1;
            }
            
            #pragma omp section
            {
                z = x + 2;
            }
        }
    }
    
    /* Return to avoid compiler warnings */
    return 0;
}
