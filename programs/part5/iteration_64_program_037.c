/* test_depend_clauses.c - Exhaustive test for OMP_CLAUSE_DEPEND_* pretty-printing */
/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test_depend_clauses.c */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* Helper to prevent "unused variable" warnings */
    volatile int use = 0;
    
    /* ===== BLOCK 1: Basic depend clause types in task construct ===== */
    if (0) {  /* Prevent execution, focus on compilation */
        /* OMP_CLAUSE_DEPEND_IN */
        #pragma omp task depend(in: x)
        { use = x; }
        
        /* OMP_CLAUSE_DEPEND_OUT */
        #pragma omp task depend(out: y)
        { y = 1; }
        
        /* OMP_CLAUSE_DEPEND_INOUT */
        #pragma omp task depend(inout: z)
        { z = z + 1; }
        
        /* Multiple dependencies in single clause */
        #pragma omp task depend(in: x, y) depend(out: z)
        { z = x + y; }
    }
    
    /* ===== BLOCK 2: Set-based dependencies ===== */
    if (0) {
        /* OMP_CLAUSE_DEPEND_INOUTSET */
        #pragma omp task depend(inoutset: arr[0])
        { arr[0] = 1; }
        
        /* OMP_CLAUSE_DEPEND_MUTEXINOUTSET */
        #pragma omp task depend(mutexinoutset: arr[1])
        { arr[1] = 2; }
        
        /* Combined set dependencies */
        #pragma omp task depend(inoutset: x) depend(mutexinoutset: y)
        { use = x + y; }
    }
    
    /* ===== BLOCK 3: DEPOBJ dependency ===== */
    if (0) {
        /* OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        { use = depobj_var; }
        
        /* depobj with pointer */
        #pragma omp task depend(depobj: *ptr)
        { use = *ptr; }
    }
    
    /* ===== BLOCK 4: Target regions with dependencies ===== */
    if (0) {
        /* Target with in dependency */
        #pragma omp target depend(in: x) map(tofrom: x)
        { x = x * 2; }
        
        /* Target with out dependency and array element */
        #pragma omp target depend(out: arr[2]) map(tofrom: arr[2])
        { arr[2] = 3; }
        
        /* Target with inoutset dependency */
        #pragma omp target depend(inoutset: y) map(tofrom: y)
        { y = y + 1; }
    }
    
    /* ===== BLOCK 5: Complex expressions and iterator ===== */
    if (0) {
        int len = 5;
        
        /* Iterator modifier (C/C++ syntax) */
        #pragma omp task depend(in: arr[0:len])
        { use = arr[0]; }
        
        /* Pointer dereference */
        int *arr_ptr = arr;
        #pragma omp task depend(out: *arr_ptr)
        { *arr_ptr = 10; }
        
        /* Complex array indexing */
        #pragma omp task depend(in: arr[x+y])
        { use = arr[x+y]; }
    }
    
    /* ===== BLOCK 6: Nested constructs for deeper tree printing ===== */
    if (0) {
        #pragma omp parallel
        {
            /* Multiple tasks with different dependencies */
            #pragma omp task depend(in: x)
            { use = x; }
            
            #pragma omp task depend(out: y)
            { y = 2; }
            
            #pragma omp task depend(inoutset: z)
            { z = 3; }
            
            #pragma omp taskwait
            
            /* Task with depobj inside parallel region */
            #pragma omp task depend(depobj: depobj_var)
            { use = depobj_var; }
        }
    }
    
    /* ===== BLOCK 7: Combined constructs ===== */
    if (0) {
        /* Combined target parallel with dependency */
        #pragma omp target parallel for depend(in: x) map(tofrom: x)
        for (int i = 0; i < 10; i++) {
            x += i;
        }
        
        /* Parallel sections with task dependencies */
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                #pragma omp task depend(out: arr[0])
                { arr[0] = 100; }
            }
            
            #pragma omp section
            {
                #pragma omp task depend(in: arr[0]) depend(out: arr[1])
                { arr[1] = arr[0] + 1; }
            }
        }
    }
    
    /* ===== BLOCK 8: Multiple depend clauses to trigger OMP_CLAUSE_DEPEND_LAST ===== */
    if (0) {
        /* This should create a clause list where iteration reaches the end */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(inoutset: arr[0]) depend(mutexinoutset: arr[1]) \
                         depend(depobj: depobj_var)
        {
            /* Complex operation using all variables */
            depobj_var = x + y + z + arr[0] + arr[1];
            use = depobj_var;
        }
    }
    
    /* ===== BLOCK 9: Taskloop with dependencies ===== */
    if (0) {
        #pragma omp taskloop depend(in: x) depend(out: arr[0:5])
        for (int i = 0; i < 10; i++) {
            arr[i] = x + i;
        }
    }
    
    return 0;
}
