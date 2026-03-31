/* test_depend_clauses.c - Exhaustive test for OMP_CLAUSE_DEPEND_* pretty-printing */
/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test_depend_clauses.c */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* 1. Basic depend clause kinds in simple tasks */
    /* Each if(0) block ensures compilation without execution */
    
    /* OMP_CLAUSE_DEPEND_IN */
    if (0) {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
    
    /* OMP_CLAUSE_DEPEND_OUT */
    if (0) {
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
    }
    
    /* OMP_CLAUSE_DEPEND_INOUT */
    if (0) {
        #pragma omp task depend(inout: z)
        {
            z = z + 1;
        }
    }
    
    /* 2. Set-based dependency types */
    /* OMP_CLAUSE_DEPEND_INOUTSET */
    if (0) {
        #pragma omp task depend(inoutset: arr[0])
        {
            arr[0] = 10;
        }
    }
    
    /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
    if (0) {
        #pragma omp task depend(mutexinoutset: arr[1])
        {
            arr[1] = 20;
        }
    }
    
    /* 3. DEPOBJ clause - specific to depend object */
    /* OMP_CLAUSE_DEPEND_DEPOBJ */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 30;
        }
    }
    
    /* 4. Complex dependency expressions */
    if (0) {
        int a = 0, b = 0, c = 0;
        int *dyn_ptr = malloc(sizeof(int));
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: a, b) depend(out: c)
        {
            c = a + b;
        }
        
        /* Pointer dereference */
        #pragma omp task depend(out: *dyn_ptr)
        {
            *dyn_ptr = 42;
        }
        
        /* Array section (iterator modifier) */
        int len = 5;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i;
            }
        }
        
        free(dyn_ptr);
    }
    
    /* 5. Target regions with dependencies */
    if (0) {
        int target_var = 0;
        
        /* Target with depend clause */
        #pragma omp target depend(out: target_var) map(tofrom: target_var)
        {
            target_var = 100;
        }
        
        /* Combined construct */
        #pragma omp target parallel for depend(in: arr) map(tofrom: arr[0:10])
        for (int i = 0; i < 10; i++) {
            arr[i] = i * 2;
        }
    }
    
    /* 6. Nested constructs for complex printing paths */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Multiple depend clauses on single task - may trigger iteration to LAST */
                #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                                 depend(mutexinoutset: arr[2]) depend(inoutset: arr[3])
                {
                    y = x + z;
                    arr[2] = arr[3] + 1;
                }
                
                /* Dependency chain */
                #pragma omp task depend(out: x)
                { x = 1; }
                
                #pragma omp task depend(in: x) depend(out: y)
                { y = x + 1; }
                
                #pragma omp task depend(in: y)
                { z = y * 2; }
            }
        }
    }
    
    /* 7. DEPOBJ with pointer */
    if (0) {
        int depobj_target = 0;
        int *depobj_ptr = &depobj_target;
        
        #pragma omp task depend(depobj: *depobj_ptr)
        {
            depobj_target = 99;
        }
    }
    
    /* 8. Mixed clause types to ensure all are visited */
    /* This block combines all depend kinds to potentially trigger OMP_CLAUSE_DEPEND_LAST */
    if (0) {
        int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0, v6 = 0;
        
        #pragma omp task \
            depend(depobj: v1) \
            depend(in: v2) \
            depend(out: v3) \
            depend(inout: v4) \
            depend(mutexinoutset: v5) \
            depend(inoutset: v6)
        {
            /* All depend kinds in one construct */
            v3 = v2 + v4;
            v5 = v6;
        }
    }
    
    return 0;
}
