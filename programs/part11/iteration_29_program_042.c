/* test_omp_clauses.c
 * Designed to trigger pretty-printing of OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in GCC's tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_clauses.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Function targeted with 'for' clause in declare target */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some computation */
        volatile int x = i * i;
    }
}

/* Function targeted with 'parallel' clause in declare target */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel
    {
        #pragma omp for
        for (i = 0; i < n; i++) {
            volatile int y = i + 1;
        }
    }
}

/* Function targeted with 'sections' clause in declare target */
#pragma omp declare target to(func_sections) sections
void func_sections(void) {
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            volatile int a = 1;
        }
        #pragma omp section
        {
            volatile int b = 2;
        }
    }
}

/* Function targeted with combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel sections
void func_combined(int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            #pragma omp sections
            {
                #pragma omp section
                { volatile int x = i * 2; }
                #pragma omp section  
                { volatile int y = i * 3; }
            }
        }
    }
}

/* Function using taskgroup construct */
void func_taskgroup(int n) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                { volatile int t1 = 1; }
                #pragma omp task
                { volatile int t2 = 2; }
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare variant(func_for) match(construct={dispatch}, device={arch(any)})
void func_dispatch(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        volatile int z = i * i;
    }
}

/* Main function to ensure all functions are referenced */
int main(void) {
    int n = 100;
    
    /* Call all functions to ensure they're compiled */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_combined(n);
    func_taskgroup(n);
    func_dispatch(n);
    
    /* Additional OpenMP constructs for context */
    #pragma omp target teams distribute parallel for map(tofrom:n)
    for (int i = 0; i < n; i++) {
        volatile int w = i * 4;
    }
    
    /* Nested parallel region with sections */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { func_for(10); }
            #pragma omp section
            { func_parallel(10); }
        }
    }
    
    return 0;
}
