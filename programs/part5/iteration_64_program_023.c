/* test-omp-depend-coverage.c
 * 
 * This test program is designed to trigger GCC's tree pretty-printer
 * for all OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting
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
        { arr[0] = 10; }
        
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: arr[1])
        { arr[1] = 20; }
        
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 30; }
    }
    
    if (0) {
        /* Block 2: Multiple depend clauses in single construct 
           This helps trigger iteration through all clause types,
           potentially reaching OMP_CLAUSE_DEPEND_LAST */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[0]) depend(mutexinoutset: arr[1])
        {
            y = x;
            z = y + 1;
            arr[0] = z;
            arr[1] = arr[0] * 2;
        }
    }
    
    if (0) {
        /* Block 3: Complex dependency expressions */
        int *dynamic_ptr = malloc(sizeof(int));
        
        /* Array element with index */
        #pragma omp task depend(in: arr[x]) depend(out: arr[y])
        { arr[y] = arr[x] + 1; }
        
        /* Pointer dereference */
        #pragma omp task depend(inout: *ptr)
        { *ptr += 1; }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y, arr[2]) depend(out: z, arr[3])
        {
            z = x + y + arr[2];
            arr[3] = z;
        }
        
        free(dynamic_ptr);
    }
    
    if (0) {
        /* Block 4: Depend clauses in target regions */
        /* OMP_CLAUSE_DEPEND_IN in target */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x = x * 2; }
        
        /* OMP_CLAUSE_DEPEND_OUT in target */
        #pragma omp target depend(out: arr[0]) map(tofrom: arr[0:1])
        { arr[0] = 100; }
        
        /* Multiple depend types in target */
        #pragma omp target depend(in: x) depend(out: y) depend(inout: z)
        {
            y = x;
            z = y * 2;
        }
    }
    
    if (0) {
        /* Block 5: Nested constructs for complex pretty-printing paths */
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Task with mutexinoutset inside parallel region */
                #pragma omp task depend(mutexinoutset: x)
                { x = 42; }
                
                /* Task with inoutset inside parallel region */
                #pragma omp task depend(inoutset: y)
                { y = 24; }
                
                /* Task with depobj inside parallel region */
                #pragma omp task depend(depobj: z)
                { z = x + y; }
            }
        }
    }
    
    if (0) {
        /* Block 6: Combined constructs */
        /* Combined target parallel with depend */
        #pragma omp target parallel for depend(in: arr[0:5]) map(tofrom: arr[0:10])
        for (int i = 0; i < 10; i++) {
            arr[i] = i * 2;
        }
        
        /* Combined taskloop with depend */
        #pragma omp taskloop depend(inout: x) shared(arr)
        for (int i = 0; i < 10; i++) {
            arr[i] += x;
        }
    }
    
    if (0) {
        /* Block 7: Iterator modifier (C/C++ specific) */
        int len = 10;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i;
            }
        }
        
        /* Iterator with stride */
        #pragma omp task depend(out: arr[0:len:2])
        {
            for (int i = 0; i < len; i += 2) {
                arr[i] = i * 2;
            }
        }
    }
    
    /* Ensure all tasks complete if actually executed */
    #pragma omp taskwait
    
    return 0;
}
