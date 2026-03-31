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
    
    /* Block 2: Multiple depend clauses on single construct (triggers iteration) */
    if (0) {
        /* Multiple clauses force pretty-printer to iterate through all,
           potentially reaching internal list boundaries */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[2]) depend(mutexinoutset: arr[3])
        {
            z = x + y;
            arr[2] = z;
            arr[3] = z * 2;
        }
        
        #pragma omp taskwait
    }
    
    /* Block 3: Depend clause with depobj modifier */
    if (0) {
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj)
        { x = 100; }
        
        #pragma omp taskwait
    }
    
    /* Block 4: Complex dependency expressions */
    if (0) {
        int *dyn_arr = malloc(100 * sizeof(int));
        
        /* Array element with index expression */
        #pragma omp task depend(in: arr[x]) depend(out: arr[y+1])
        { arr[y+1] = arr[x] * 2; }
        
        /* Pointer dereference */
        #pragma omp task depend(inout: *ptr)
        { *ptr += 10; }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y, z) depend(out: arr[4], arr[5])
        {
            arr[4] = x + y;
            arr[5] = z;
        }
        
        free(dyn_arr);
        #pragma omp taskwait
    }
    
    /* Block 5: Depend in target regions */
    if (0) {
        /* OMP_CLAUSE_DEPEND_IN in target */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x = x * 2; }
        
        /* OMP_CLAUSE_DEPEND_OUT in target */
        #pragma omp target depend(out: y) map(tofrom: y)
        { y = 100; }
        
        /* Multiple depend types in target */
        #pragma omp target depend(in: x) depend(out: arr[6]) map(tofrom: x, arr[6])
        { arr[6] = x; }
    }
    
    /* Block 6: Nested constructs with dependencies */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                /* Task with inoutset inside parallel region */
                #pragma omp task depend(inoutset: arr[7])
                { arr[7] = 70; }
                
                /* Task with mutexinoutset inside parallel region */
                #pragma omp task depend(mutexinoutset: arr[8])
                { arr[8] = 80; }
                
                /* Dependent task chain */
                #pragma omp task depend(out: arr[9])
                { arr[9] = 1; }
                
                #pragma omp task depend(in: arr[9])
                { arr[9] = arr[9] * 2; }
                
                #pragma omp task depend(in: arr[9])
                { arr[9] = arr[9] + 1; }
            }
        }
    }
    
    /* Block 7: Combined constructs with depend */
    if (0) {
        /* Combined target parallel with depend */
        #pragma omp target parallel for depend(in: x) map(tofrom: x, arr[0:5])
        for (int i = 0; i < 5; i++) {
            arr[i] = x + i;
        }
        
        /* Taskloop with depend */
        #pragma omp taskloop depend(inout: y) shared(arr)
        for (int i = 0; i < 10; i++) {
            arr[i] += y;
        }
    }
    
    /* Block 8: Iterator modifier (C/C++ specific) */
    if (0) {
        /* Depend with iterator for array section */
        #pragma omp task depend(in: arr[0:5])  /* Array section */
        {
            int sum = 0;
            for (int i = 0; i < 5; i++) {
                sum += arr[i];
            }
            x = sum;
        }
        
        #pragma omp taskwait
    }
    
    /* Block 9: Mixed clause types to ensure full enumeration */
    if (0) {
        /* This should trigger pretty-printing of all depend types in one go */
        void *depobj2 = NULL;
        
        #pragma omp task depend(depobj: depobj2)
        { /* depobj */ }
        
        #pragma omp task depend(in: x)
        { /* in */ }
        
        #pragma omp task depend(out: y)
        { /* out */ }
        
        #pragma omp task depend(inout: z)
        { /* inout */ }
        
        #pragma omp task depend(mutexinoutset: arr[0])
        { /* mutexinoutset */ }
        
        #pragma omp task depend(inoutset: arr[1])
        { /* inoutset */ }
        
        #pragma omp taskwait
    }
    
    return 0;
}
