/* test-omp-depend-coverage.c
 * 
 * This test is designed to trigger GCC's tree pretty-printer for all
 * variants of the OpenMP `depend` clause. The goal is to ensure the
 * uncovered block in tree-pretty-print.cc (lines 824-846) is executed
 * when compiler dump flags are enabled.
 *
 * Compile with:
 *   gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 *   gcc -O2 -fopenmp -fdump-tree-optimized -fdump-tree-original -c test-omp-depend-coverage.c
 *   gcc -O0 -fopenmp -foffload=disable -fdump-tree-omplower -c test-omp-depend-coverage.c
 *
 * The program does not need to execute correctly; compilation with
 * tree dumping enabled is sufficient.
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency expressions */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int len = 10;
    
    /* Use conditional blocks to isolate test cases while ensuring
     * all code is parsed by the compiler */
    
    /* ===== Block 1: Basic depend clause types in tasks ===== */
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
        { arr[0] = 5; }
        
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: arr[1])
        { arr[1] = 6; }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y) depend(out: z)
        { z = x + y; }
    }
    
    /* ===== Block 2: depend(depobj) clause ===== */
    if (0) {
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: ptr)
        { *ptr = 10; }
    }
    
    /* ===== Block 3: Target regions with dependencies ===== */
    if (0) {
        /* OMP_CLAUSE_DEPEND_IN in target */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x = x * 2; }
        
        /* OMP_CLAUSE_DEPEND_OUT in target */
        #pragma omp target depend(out: arr[2]) map(tofrom: arr[2])
        { arr[2] = 20; }
        
        /* Combined construct with dependency */
        #pragma omp target parallel for depend(inout: arr[3]) map(tofrom: arr[3:1])
        for (int i = 0; i < 1; i++) {
            arr[3] = 30;
        }
    }
    
    /* ===== Block 4: Complex dependency expressions ===== */
    if (0) {
        /* Pointer dereference */
        int *p = arr;
        #pragma omp task depend(out: *p)
        { *p = 100; }
        
        /* Array section with iterator (C/C++ syntax) */
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) arr[i] = i;
        }
        
        /* Multiple complex dependencies */
        #pragma omp task depend(in: arr[0], arr[5]) depend(out: *ptr, arr[9])
        {
            *ptr = arr[0] + arr[5];
            arr[9] = 99;
        }
    }
    
    /* ===== Block 5: Nested constructs for deeper tree printing ===== */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Mix of dependency types in nested tasks */
                #pragma omp task depend(in: x)
                { x = 1; }
                
                #pragma omp task depend(out: y)
                { y = 2; }
                
                #pragma omp task depend(inout: z) depend(inoutset: arr[4])
                { z = x + y; arr[4] = z; }
                
                /* This should help trigger OMP_CLAUSE_DEPEND_LAST
                 * through clause list iteration */
                #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                                 depend(mutexinoutset: arr[5]) depend(inoutset: arr[6])
                {
                    z = x + y + arr[5] + arr[6];
                }
            }
        }
    }
    
    /* ===== Block 6: Taskwait with depend ===== */
    if (0) {
        #pragma omp task depend(inout: x)
        { x = 50; }
        
        #pragma omp taskwait depend(inoutset: x)
        
        #pragma omp task depend(out: y)
        { y = x + 10; }
    }
    
    /* ===== Block 7: Taskloop with dependencies ===== */
    if (0) {
        #pragma omp taskloop depend(in: arr[0]) depend(out: arr[1:5])
        for (int i = 0; i < 10; i++) {
            arr[i] = i * 2;
        }
    }
    
    /* Prevent "unused variable" warnings */
    (void)x; (void)y; (void)z; (void)arr; (void)ptr; (void)len;
    
    return 0;
}
