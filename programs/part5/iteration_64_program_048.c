/* test-omp-depend-coverage.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Block 1: Basic depend clause types in task construct */
    if (0) {
        #pragma omp task depend(in: x)
        { x = 1; }
        
        #pragma omp task depend(out: y)
        { y = 2; }
        
        #pragma omp task depend(inout: z)
        { z = x + y; }
        
        #pragma omp taskwait
    }
    
    /* Block 2: Set-based depend types in nested tasks */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(mutexinoutset: x)
                { x++; }
                
                #pragma omp task depend(inoutset: y)
                { y++; }
                
                #pragma omp task depend(inoutset: x, y)  /* Multiple items */
                { z = x + y; }
            }
        }
    }
    
    /* Block 3: depobj modifier - specific to depend clause */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 42; }
        
        #pragma omp taskwait
    }
    
    /* Block 4: Complex dependency expressions */
    if (0) {
        int *dynamic_arr = malloc(100 * sizeof(int));
        
        #pragma omp task depend(in: arr[0])  /* Array element */
        { arr[0] = 10; }
        
        #pragma omp task depend(out: *ptr)  /* Pointer dereference */
        { *ptr = 20; }
        
        #pragma omp task depend(in: arr[1], arr[2]) depend(out: arr[3])  /* Multiple clauses */
        { arr[3] = arr[1] + arr[2]; }
        
        free(dynamic_arr);
    }
    
    /* Block 5: Iterator modifier (C/C++ specific) */
    if (0) {
        int len = 10;
        
        #pragma omp task depend(in: arr[0:len])  /* Array section with iterator */
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i;
            }
        }
        
        #pragma omp taskwait
    }
    
    /* Block 6: Target regions with depend clauses */
    if (0) {
        #pragma omp target depend(in: x) depend(out: y) map(tofrom: x, y)
        {
            y = x * 2;
        }
        
        #pragma omp target teams distribute parallel for depend(out: arr[0:10]) map(tofrom: arr)
        for (int i = 0; i < 10; i++) {
            arr[i] = i * i;
        }
    }
    
    /* Block 7: Combined constructs */
    if (0) {
        #pragma omp target parallel for depend(inout: x) map(tofrom: x)
        for (int i = 0; i < 10; i++) {
            x += i;
        }
    }
    
    /* Block 8: Multiple depend clauses to potentially trigger LAST iteration */
    if (0) {
        /* This creates a chain of dependencies that might exercise the full iteration */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(mutexinoutset: arr[0]) depend(inoutset: arr[1])
        {
            y = x + z;
            arr[0] = arr[1] + 1;
        }
    }
    
    /* Block 9: Task with depend and other clauses */
    if (0) {
        #pragma omp task default(none) shared(x, y, z) private(arr) \
                   depend(inoutset: x) if(1) final(0) mergeable priority(1)
        {
            x = y + z;
        }
    }
    
    return 0;
}
