/* test-omp-depend-coverage.c
 * 
 * This test is designed to trigger the pretty-printing logic for all
 * OMP_CLAUSE_DEPEND_* enumeration values in GCC's tree-pretty-print.cc.
 * Compile with flags like: -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple
 * The program doesn't need to execute correctly; the goal is to generate
 * compiler dumps that show the clause names being printed.
 */

#include <stdlib.h>

int main(void) {
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
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y) depend(out: z)
        { z = x + y; }
    }
    
    /* Block 2: Set-based dependency types */
    if (0) {
        /* OMP_CLAUSE_DEPEND_INOUTSET */
        #pragma omp task depend(inoutset: arr[0])
        { arr[0] = 5; }
        
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: arr[1])
        { arr[1] = 6; }
        
        /* Combined set dependencies */
        #pragma omp task depend(inoutset: arr[0], arr[1]) \
                         depend(mutexinoutset: arr[2])
        { arr[2] = arr[0] + arr[1]; }
    }
    
    /* Block 3: depobj dependency type */
    if (0) {
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        { depobj_var = 42; }
        
        /* depobj with pointer */
        #pragma omp task depend(depobj: *ptr)
        { *ptr = 100; }
    }
    
    /* Block 4: Complex dependency expressions */
    if (0) {
        /* Array element with index */
        #pragma omp task depend(out: arr[x])
        { arr[x] = 10; }
        
        /* Pointer dereference */
        int *p = arr;
        #pragma omp task depend(in: *p)
        { y = *p; }
        
        /* Iterator modifier (C/C++ syntax) */
        int len = 5;
        #pragma omp task depend(in: arr[0:len])
        {
            for (int i = 0; i < len; i++) arr[i] = i;
        }
    }
    
    /* Block 5: Target regions with dependencies */
    if (0) {
        /* OMP_CLAUSE_DEPEND_IN in target */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x = x * 2; }
        
        /* OMP_CLAUSE_DEPEND_OUT in target */
        #pragma omp target depend(out: arr) map(tofrom: arr[0:10])
        {
            for (int i = 0; i < 10; i++) arr[i] = i;
        }
        
        /* Combined construct */
        #pragma omp target parallel for depend(inout: x)
        for (int i = 0; i < 10; i++) {
            arr[i] = x + i;
        }
    }
    
    /* Block 6: Nested constructs for complex printing paths */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Multiple depend clauses to potentially trigger LAST iteration */
                #pragma omp task depend(in: x) depend(out: y) \
                                 depend(inout: z) depend(inoutset: arr[0])
                {
                    y = x;
                    z = y + 1;
                    arr[0] = z;
                }
                
                /* Another task with mixed dependencies */
                #pragma omp task depend(in: y) depend(mutexinoutset: arr[1]) \
                                 depend(depobj: depobj_var)
                {
                    arr[1] = y;
                    depobj_var = arr[1];
                }
            }
        }
    }
    
    /* Block 7: Taskwait with depend */
    if (0) {
        #pragma omp task depend(inout: x)
        { x = 1; }
        
        #pragma omp taskwait depend(inoutset: x)
        
        #pragma omp task depend(in: x)
        { y = x; }
    }
    
    /* Block 8: Taskloop with dependencies */
    if (0) {
        #pragma omp taskloop depend(in: x) depend(out: arr)
        for (int i = 0; i < 10; i++) {
            arr[i] = x + i;
        }
    }
    
    /* Block 9: Taskgroup with task dependencies */
    if (0) {
        #pragma omp taskgroup
        {
            #pragma omp task depend(inout: x)
            { x = 10; }
            
            #pragma omp task depend(in: x) depend(out: y)
            { y = x * 2; }
        }
    }
    
    /* Force a barrier to ensure all tasks are considered */
    #pragma omp taskwait
    
    return 0;
}
