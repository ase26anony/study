/* Test program to exercise GCC's OpenMP depend clause pretty-printing */
/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c */

#include <stdlib.h>

int main(void) {
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Block 1: Basic depend clause types in tasks */
    if (0) {
        #pragma omp task depend(in: x)
        { x = 1; }
        
        #pragma omp task depend(out: y)
        { y = 2; }
        
        #pragma omp task depend(inout: z)
        { z = x + y; }
        
        #pragma omp task depend(inoutset: arr[0])
        { arr[0] = 5; }
        
        #pragma omp task depend(mutexinoutset: arr[1])
        { arr[1] = 6; }
        
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 7; }
    }
    
    /* Block 2: Multiple depend clauses on single construct (triggers iteration) */
    if (0) {
        #pragma omp task depend(in: x, y) depend(out: z) depend(inout: arr[2])
        {
            z = x + y;
            arr[2] = z;
        }
    }
    
    /* Block 3: Target regions with depend clauses */
    if (0) {
        #pragma omp target depend(in: arr[0]) depend(out: arr[1]) map(tofrom: arr[0:2])
        {
            arr[1] = arr[0] * 2;
        }
        
        #pragma omp target data map(tofrom: arr[3:2])
        #pragma omp target depend(inoutset: arr[3]) depend(in: arr[4])
        {
            arr[3] += arr[4];
        }
    }
    
    /* Block 4: Parallel region with nested dependent tasks */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(in: x)
                { arr[5] = x; }
                
                #pragma omp task depend(out: y) depend(inoutset: arr[6])
                { 
                    y = 10;
                    arr[6] = y;
                }
                
                #pragma omp task depend(mutexinoutset: arr[7]) depend(depobj: depobj_var)
                { arr[7] = depobj_var; }
            }
        }
    }
    
    /* Block 5: Combined construct with iterator modifier (C/C++) */
    if (0) {
        int len = 5;
        #pragma omp task depend(in: arr[0:len]) depend(out: arr[5:len])
        {
            for (int i = 0; i < len; i++) {
                arr[5 + i] = arr[i] * 2;
            }
        }
    }
    
    /* Block 6: Complex pointer expressions */
    if (0) {
        int **pptr = &ptr;
        #pragma omp task depend(in: *ptr) depend(out: **pptr) depend(inout: arr[*ptr])
        {
            **pptr = 42;
            arr[**pptr % 10] = 99;
        }
    }
    
    /* Block 7: Taskwait with depend clause */
    if (0) {
        #pragma omp task depend(out: x)
        { x = 1; }
        
        #pragma omp task depend(in: x) depend(out: y)
        { y = x + 1; }
        
        #pragma omp taskwait depend(inoutset: x)  /* taskwait with depend */
        
        #pragma omp task depend(in: y)
        { z = y * 2; }
    }
    
    /* Block 8: Taskloop with depend */
    if (0) {
        #pragma omp taskloop depend(in: arr[0]) depend(out: arr[9]) nogroup
        for (int i = 0; i < 10; i++) {
            arr[9] += arr[i];
        }
    }
    
    /* Block 9: All depend types in one complex construct */
    if (0) {
        #pragma omp target parallel for depend(in: x) \
                depend(out: y) depend(inout: z) \
                depend(mutexinoutset: arr[0]) \
                depend(inoutset: arr[1]) \
                depend(depobj: depobj_var) \
                map(tofrom: x, y, z, arr[0:2], depobj_var)
        for (int i = 0; i < 10; i++) {
            x += i;
            y += x;
            z = x + y;
            arr[0] = z;
            arr[1] = depobj_var;
        }
    }
    
    return 0;
}
