/* test-omp-depend-coverage.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for all dependency types */
    int x, y, z, w;
    int arr[10];
    int *ptr = &x;
    int depobj_var;
    
    /* 1. Basic depend(in:) clause */
    if (0) {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
    
    /* 2. Basic depend(out:) clause */
    if (0) {
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
    }
    
    /* 3. Basic depend(inout:) clause */
    if (0) {
        #pragma omp task depend(inout: z)
        {
            z++;
        }
    }
    
    /* 4. depend(inoutset:) clause - set-based dependency */
    if (0) {
        #pragma omp task depend(inoutset: w)
        {
            w = 4;
        }
    }
    
    /* 5. depend(mutexinoutset:) clause - mutual exclusion set */
    if (0) {
        #pragma omp task depend(mutexinoutset: arr[0])
        {
            arr[0] = 5;
        }
    }
    
    /* 6. depend(depobj:) clause - dependency object */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 6;
        }
    }
    
    /* 7. Multiple depend clauses on single construct (triggers iteration through all) */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z)
        {
            y = x + z;
        }
    }
    
    /* 8. Complex expressions in depend clauses */
    if (0) {
        #pragma omp task depend(in: arr[2]) depend(out: *ptr)
        {
            *ptr = arr[2] * 2;
        }
    }
    
    /* 9. depend clause in target region */
    if (0) {
        #pragma omp target depend(in: x) depend(out: arr[0:5])
        {
            arr[0] = x;
        }
    }
    
    /* 10. depend clause in combined construct */
    if (0) {
        #pragma omp target parallel for depend(out: arr[0:10])
        for (int i = 0; i < 10; i++) {
            arr[i] = i;
        }
    }
    
    /* 11. Nested tasks with dependencies inside parallel region */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(out: x)
                { x = 1; }
                
                #pragma omp task depend(in: x) depend(out: y)
                { y = x + 1; }
                
                #pragma omp task depend(inoutset: z) depend(mutexinoutset: w)
                { 
                    z = y + 1;
                    w = z * 2;
                }
            }
        }
    }
    
    /* 12. Iterator modifier with array section (C/C++ syntax) */
    if (0) {
        int len = 5;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) {
                arr[i] = i * 2;
            }
        }
    }
    
    /* 13. Mixed dependency types including depobj */
    if (0) {
        #pragma omp task depend(depobj: depobj_var) depend(in: x) depend(out: y) \
                         depend(inout: z) depend(mutexinoutset: w) depend(inoutset: arr[0])
        {
            y = x + z + w + arr[0];
            depobj_var = y;
        }
    }
    
    /* 14. Task with if clause and depend - testing clause ordering */
    if (0) {
        int condition = 1;
        #pragma omp task if(condition) depend(inout: x) priority(5)
        {
            x *= 2;
        }
    }
    
    /* 15. Taskwait with depend clause */
    if (0) {
        #pragma omp task depend(out: x)
        { x = 10; }
        
        #pragma omp taskwait depend(in: x)
    }
    
    /* 16. Taskloop with depend clause */
    if (0) {
        #pragma omp taskloop depend(in: x) grainsize(10)
        for (int i = 0; i < 100; i++) {
            arr[i % 10] += i;
        }
    }
    
    /* 17. Target enter/exit data with depend */
    if (0) {
        #pragma omp target enter data map(to: arr[0:10]) depend(inout: x)
        
        #pragma omp target exit data map(from: arr[0:10]) depend(inout: x)
    }
    
    return 0;
}
