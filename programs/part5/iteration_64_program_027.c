/* test-omp-depend-coverage.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    void *depobj = NULL;
    
    /* Block 1: Basic depend clause types in tasks */
    if (0) {
        #pragma omp task depend(in: x)
        { x = 1; }
        
        #pragma omp task depend(out: y)
        { y = 2; }
        
        #pragma omp task depend(inout: z)
        { z = x + y; }
        
        #pragma omp taskwait
    }
    
    /* Block 2: Set-based depend clauses */
    if (0) {
        #pragma omp task depend(mutexinoutset: x)
        { x *= 2; }
        
        #pragma omp task depend(inoutset: y)
        { y += 3; }
        
        #pragma omp taskwait
    }
    
    /* Block 3: depobj clause type */
    if (0) {
        #pragma omp task depend(depobj: depobj)
        { depobj = &x; }
        
        #pragma omp taskwait
    }
    
    /* Block 4: Multiple depend clauses on single construct (triggers iteration) */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z)
        {
            z = x;
            y = z + 1;
        }
        
        #pragma omp taskwait
    }
    
    /* Block 5: Complex dependency expressions */
    if (0) {
        #pragma omp task depend(in: arr[0]) depend(out: *ptr)
        {
            *ptr = arr[0] + 5;
        }
        
        int len = 10;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) arr[i] = i;
        }
        
        #pragma omp taskwait
    }
    
    /* Block 6: Target regions with depend clauses */
    if (0) {
        #pragma omp target depend(in: x) depend(out: arr[0])
        {
            arr[0] = x;
        }
        
        #pragma omp target teams distribute parallel for depend(out: arr[0:10])
        for (int i = 0; i < 10; i++) {
            arr[i] = i * 2;
        }
    }
    
    /* Block 7: Nested tasks in parallel region */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(inoutset: x)
                { x++; }
                
                #pragma omp task depend(inoutset: x)
                { x--; }
                
                #pragma omp task depend(mutexinoutset: y)
                { y = x; }
            }
        }
    }
    
    /* Block 8: Combined construct with depend */
    if (0) {
        #pragma omp target parallel for depend(in: x) map(tofrom: arr)
        for (int i = 0; i < 10; i++) {
            arr[i] += x;
        }
    }
    
    /* Block 9: All depend types in sequence (ensures full enumeration) */
    if (0) {
        /* This should trigger pretty-printing of all clause types */
        #pragma omp task depend(depobj: depobj)
        { /* empty */ }
        
        #pragma omp task depend(in: x)
        { /* empty */ }
        
        #pragma omp task depend(out: y)
        { /* empty */ }
        
        #pragma omp task depend(inout: z)
        { /* empty */ }
        
        #pragma omp task depend(mutexinoutset: x)
        { /* empty */ }
        
        #pragma omp task depend(inoutset: y)
        { /* empty */ }
        
        #pragma omp taskwait
    }
    
    /* Block 10: Iterator modifier (C/C++ specific) */
    if (0) {
        int start = 0, end = 5;
        #pragma omp task depend(iterator(i=0:5), in: arr[i])
        {
            for (int i = start; i < end; i++) {
                arr[i] = i;
            }
        }
        
        #pragma omp taskwait
    }
    
    return 0;
}
