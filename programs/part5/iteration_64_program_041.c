/* test_omp_depend_coverage.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test_omp_depend_coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for all dependency types */
    int x, y, z, w;
    int arr[10];
    int *ptr = &x;
    int depobj_var;
    
    /* Prevent "unused variable" warnings */
    (void)x; (void)y; (void)z; (void)w;
    (void)arr; (void)ptr; (void)depobj_var;
    
    /* ============================================
     * BLOCK 1: Exhaustive depend clause enumeration
     * Each if(0) block isolates a test case but ensures compilation
     * ============================================ */
    
    /* 1. depend(depobj: ...) - OMP_CLAUSE_DEPEND_DEPOBJ */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            /* Empty task body - we only care about clause printing */
        }
    }
    
    /* 2. depend(in: ...) - OMP_CLAUSE_DEPEND_IN */
    if (0) {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
    
    /* 3. depend(out: ...) - OMP_CLAUSE_DEPEND_OUT */
    if (0) {
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
    }
    
    /* 4. depend(inout: ...) - OMP_CLAUSE_DEPEND_INOUT */
    if (0) {
        #pragma omp task depend(inout: z)
        {
            z++;
        }
    }
    
    /* 5. depend(mutexinoutset: ...) - OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
    if (0) {
        #pragma omp task depend(mutexinoutset: w)
        {
            w = 5;
        }
    }
    
    /* 6. depend(inoutset: ...) - OMP_CLAUSE_DEPEND_INOUTSET */
    if (0) {
        #pragma omp task depend(inoutset: arr[0])
        {
            arr[0] = 10;
        }
    }
    
    /* ============================================
     * BLOCK 2: Multiple depend clauses on single construct
     * This may help trigger OMP_CLAUSE_DEPEND_LAST iteration
     * ============================================ */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(mutexinoutset: w) depend(inoutset: arr[1])
        {
            x = y + z;
        }
    }
    
    /* ============================================
     * BLOCK 3: Complex dependency expressions
     * ============================================ */
    if (0) {
        /* Array element with index */
        #pragma omp task depend(in: arr[2])
        {
            arr[2] = 20;
        }
        
        /* Pointer dereference */
        #pragma omp task depend(out: *ptr)
        {
            *ptr = 30;
        }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y) depend(out: z, w)
        {
            z = x + y;
            w = x - y;
        }
        
        /* Iterator modifier (C/C++ specific) */
        int len = 5;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i;
            }
        }
    }
    
    /* ============================================
     * BLOCK 4: Different OpenMP constructs with depend clauses
     * ============================================ */
    
    /* 4a: OpenMP Target region */
    if (0) {
        #pragma omp target depend(inout: x) map(tofrom: x)
        {
            x *= 2;
        }
    }
    
    /* 4b: Combined construct */
    if (0) {
        #pragma omp target parallel for depend(out: arr[0:5])
        for (int i = 0; i < 5; i++) {
            arr[i] = i * i;
        }
    }
    
    /* 4c: Taskwait with depend */
    if (0) {
        #pragma omp task depend(out: x)
        { x = 1; }
        
        #pragma omp task depend(in: x)
        { y = x + 1; }
        
        #pragma omp taskwait depend(inoutset: x)
    }
    
    /* 4d: Taskloop with depend */
    if (0) {
        #pragma omp taskloop depend(inout: x) nogroup
        for (int i = 0; i < 10; i++) {
            arr[i] = x + i;
        }
    }
    
    /* ============================================
     * BLOCK 5: Nested parallelism with dependencies
     * ============================================ */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Producer task with depobj */
                #pragma omp task depend(depobj: depobj_var)
                {
                    depobj_var = 100;
                }
                
                /* Consumer task with mutexinoutset */
                #pragma omp task depend(mutexinoutset: depobj_var)
                {
                    depobj_var *= 2;
                }
                
                /* Another task with inoutset */
                #pragma omp task depend(inoutset: depobj_var)
                {
                    depobj_var += 10;
                }
            }
        }
    }
    
    /* ============================================
     * BLOCK 6: Simulated OMP_CLAUSE_DEPEND_LAST scenario
     * Multiple clauses that might cause iteration to end
     * ============================================ */
    if (0) {
        /* This creates a chain of dependencies that the pretty-printer
         * must traverse, potentially reaching the end of clause list */
        #pragma omp task depend(in: x) depend(out: y) \
                         depend(inout: z) depend(mutexinoutset: w) \
                         depend(inoutset: arr[0]) depend(depobj: depobj_var)
        {
            /* Complex operation to ensure the task isn't optimized away */
            arr[0] = x + y + z + w + depobj_var;
        }
    }
    
    /* ============================================
     * BLOCK 7: Depend on structure members
     * ============================================ */
    if (0) {
        struct container {
            int a;
            int b;
            int *c;
        } cont = {0};
        
        #pragma omp task depend(out: cont.a) depend(inout: cont.b)
        {
            cont.a = 1;
            cont.b = 2;
        }
        
        #pragma omp task depend(in: cont.a) depend(out: *cont.c)
        {
            if (cont.c) *cont.c = cont.a;
        }
    }
    
    return 0;
}
