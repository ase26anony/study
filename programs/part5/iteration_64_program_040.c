/* test-omp-depend-coverage.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare test variables */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Prevent "unused variable" warnings */
    (void)x; (void)y; (void)z; (void)arr; (void)ptr; (void)depobj_var;
    
    /* ================================ */
    /* Block 1: Basic depend clause types */
    /* ================================ */
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
    
    /* ================================ */
    /* Block 2: depend(depobj: ...) */
    /* ================================ */
    if (0) {
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 100; }
        
        /* depobj with pointer */
        #pragma omp task depend(depobj: *ptr)
        { *ptr = 200; }
    }
    
    /* ================================ */
    /* Block 3: Target regions with depend */
    /* ================================ */
    if (0) {
        /* OMP_CLAUSE_DEPEND_IN with target */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x = x * 2; }
        
        /* OMP_CLAUSE_DEPEND_OUT with target */
        #pragma omp target depend(out: arr[2]) map(tofrom: arr[2])
        { arr[2] = 30; }
        
        /* Combined construct */
        #pragma omp target parallel for depend(inout: arr[3]) map(tofrom: arr[3:1])
        for (int i = 0; i < 1; i++) {
            arr[3] = 40;
        }
    }
    
    /* ================================ */
    /* Block 4: Complex dependency expressions */
    /* ================================ */
    if (0) {
        /* Array section with iterator (C/C++ syntax) */
        int len = 5;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i;
            }
        }
        
        /* Pointer dereference */
        int *ptr2 = arr;
        #pragma omp task depend(out: *ptr2) depend(in: ptr2[1])
        {
            ptr2[0] = ptr2[1] * 2;
        }
        
        /* Multiple complex dependencies */
        #pragma omp task depend(in: arr[0], arr[1]) \
                         depend(out: arr[2], arr[3]) \
                         depend(inout: arr[4])
        {
            arr[2] = arr[0] + arr[1];
            arr[3] = arr[0] - arr[1];
            arr[4] = arr[2] * arr[3];
        }
    }
    
    /* ================================ */
    /* Block 5: Nested constructs for OMP_CLAUSE_DEPEND_LAST */
    /* ================================ */
    if (0) {
        /* Multiple depend clauses on single construct - triggers iteration to LAST */
        #pragma omp task depend(in: x) \
                         depend(out: y) \
                         depend(inout: z) \
                         depend(mutexinoutset: arr[5]) \
                         depend(inoutset: arr[6]) \
                         depend(depobj: depobj_var)
        {
            y = x + 1;
            z = y * 2;
            arr[5] = z;
            arr[6] = arr[5] + 1;
            depobj_var = arr[6];
        }
        
        /* Nested parallel with tasks */
        #pragma omp parallel
        {
            #pragma omp task depend(in: x)
            { x = x + 1; }
            
            #pragma omp task depend(out: y) depend(in: x)
            { y = x * 2; }
            
            #pragma omp task depend(inout: z) depend(in: y)
            { z = y + z; }
            
            #pragma omp taskwait
        }
    }
    
    /* ================================ */
    /* Block 6: Sections with depend */
    /* ================================ */
    if (0) {
        #pragma omp parallel sections depend(in: x)
        {
            #pragma omp section
            { y = x + 1; }
            
            #pragma omp section
            { z = x + 2; }
        }
    }
    
    /* ================================ */
    /* Block 7: Taskloop with depend */
    /* ================================ */
    if (0) {
        #pragma omp taskloop depend(inout: arr[7]) shared(arr)
        for (int i = 0; i < 10; i++) {
            arr[7] += i;
        }
    }
    
    return 0;
}
