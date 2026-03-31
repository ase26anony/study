/* test-omp-depend-coverage.c */
/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c */

#include <stdlib.h>

int main(void) {
    /* Declare variables for all dependency types */
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
        
        #pragma omp taskwait
    }
    
    /* Block 2: depend(depobj:) clause */
    if (0) {
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 100; }
        
        #pragma omp taskwait
    }
    
    /* Block 3: Multiple depend clauses on single construct (triggers iteration) */
    if (0) {
        /* Multiple clauses to ensure pretty-printer iterates through all */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[2]) depend(mutexinoutset: arr[3])
        {
            y = x;
            z = y + 1;
            arr[2] = z;
            arr[3] = arr[2] * 2;
        }
        
        #pragma omp taskwait
    }
    
    /* Block 4: Complex dependency expressions */
    if (0) {
        int *dynamic_arr = malloc(100 * sizeof(int));
        
        /* Array element with index calculation */
        #pragma omp task depend(in: arr[x]) depend(out: arr[y+1])
        { arr[y+1] = arr[x] * 2; }
        
        /* Pointer dereference */
        #pragma omp task depend(inout: *ptr)
        { *ptr += 10; }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y, z) depend(out: arr[4], arr[5])
        {
            arr[4] = x + y;
            arr[5] = z * 2;
        }
        
        #pragma omp taskwait
        free(dynamic_arr);
    }
    
    /* Block 5: depend clauses in target regions */
    if (0) {
        /* OMP_CLAUSE_DEPEND_IN in target */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x = x * 2; }
        
        /* OMP_CLAUSE_DEPEND_OUT in target */
        #pragma omp target depend(out: arr[6]) map(tofrom: arr[6])
        { arr[6] = 42; }
        
        /* Multiple dependency types in target */
        #pragma omp target depend(in: x) depend(out: y) map(tofrom: x, y)
        {
            y = x;
            x = 0;
        }
    }
    
    /* Block 6: Nested constructs for complex printing */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Task with inoutset inside parallel region */
                #pragma omp task depend(inoutset: arr[7])
                { arr[7] = 77; }
                
                /* Task with mutexinoutset inside parallel region */
                #pragma omp task depend(mutexinoutset: arr[8])
                { arr[8] = 88; }
                
                /* Dependent task chain */
                #pragma omp task depend(in: arr[7]) depend(out: arr[9])
                { arr[9] = arr[7] + 1; }
            }
        }
    }
    
    /* Block 7: Combined construct with depend clause */
    if (0) {
        /* Combined target parallel with depend */
        #pragma omp target parallel for depend(in: x) map(tofrom: arr[0:10])
        for (int i = 0; i < 10; i++) {
            arr[i] = i * x;
        }
    }
    
    /* Block 8: Iterator modifier (C/C++ specific) */
    if (0) {
        int len = 10;
        /* Using iterator modifier for array section */
        #pragma omp task depend(in: arr[0:len])
        {
            int sum = 0;
            for (int i = 0; i < len; i++) {
                sum += arr[i];
            }
            x = sum;
        }
        
        #pragma omp taskwait
    }
    
    /* Block 9: Mixed clause types to ensure full enumeration */
    if (0) {
        /* This should trigger pretty-printer to handle all enum values */
        #pragma omp task \
            depend(depobj: depobj_var) \
            depend(in: x) \
            depend(out: y) \
            depend(inout: z) \
            depend(mutexinoutset: arr[0]) \
            depend(inoutset: arr[1])
        {
            /* Complex operation using all variables */
            depobj_var = x + y + z + arr[0] + arr[1];
        }
        
        #pragma omp taskwait
    }
    
    return 0;
}
