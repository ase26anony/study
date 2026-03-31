/* test-omp-depend-coverage.c
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
        
        /* Multiple depend clauses to trigger iteration logic */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z)
        { x = y + z; }
    }
    
    /* Block 2: depobj clause type */
    if (0) {
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 100; }
        
        /* depobj with pointer */
        #pragma omp task depend(depobj: *ptr)
        { *ptr = 200; }
    }
    
    /* Block 3: Target regions with depend clauses */
    if (0) {
        /* Target with in dependency */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x *= 2; }
        
        /* Target with out dependency and array element */
        #pragma omp target depend(out: arr[2]) map(tofrom: arr[2])
        { arr[2] = 30; }
        
        /* Target with inoutset dependency */
        #pragma omp target depend(inoutset: arr[3]) map(tofrom: arr[3])
        { arr[3] = 40; }
    }
    
    /* Block 4: Parallel region with nested tasks for complex dependency graphs */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Task with mutexinoutset */
                #pragma omp task depend(mutexinoutset: y)
                { y = 50; }
                
                /* Dependent task with in */
                #pragma omp task depend(in: y)
                { z = y + 10; }
                
                /* Task with iterator modifier (C/C++ syntax) */
                #pragma omp task depend(in: arr[4:5])
                { 
                    for (int i = 4; i < 9; i++) 
                        arr[i] = i * 2; 
                }
            }
        }
    }
    
    /* Block 5: Combined constructs */
    if (0) {
        /* Combined target parallel with depend */
        #pragma omp target parallel for depend(out: arr[5]) map(tofrom: arr[5:2])
        for (int i = 5; i < 7; i++) {
            arr[i] = i * 3;
        }
    }
    
    /* Block 6: Complex dependency expressions */
    if (0) {
        int *dyn_arr = malloc(10 * sizeof(int));
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y) depend(out: z, arr[6])
        {
            z = x + y;
            arr[6] = z;
        }
        
        /* Pointer dereference in depend clause */
        #pragma omp task depend(inout: *ptr)
        {
            *ptr += 100;
        }
        
        /* Array section with runtime bounds */
        int start = 7, len = 3;
        #pragma omp task depend(out: arr[start:len])
        {
            for (int i = start; i < start + len; i++)
                arr[i] = i * 4;
        }
        
        free(dyn_arr);
    }
    
    /* Block 7: Ensure all clause types appear in taskwait depend */
    if (0) {
        #pragma omp task depend(inout: x)
        { x = 1000; }
        
        #pragma omp task depend(inoutset: y)
        { y = 2000; }
        
        #pragma omp task depend(mutexinoutset: z)
        { z = 3000; }
        
        /* taskwait with multiple depend types */
        #pragma omp taskwait depend(inout: x) depend(inoutset: y) depend(mutexinoutset: z)
    }
    
    return 0;
}
