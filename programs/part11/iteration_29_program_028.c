/* test_omp_clauses.c
 * Designed to trigger GCC's tree pretty-printer for specific OpenMP clauses:
 * - OMP_CLAUSE_FOR
 * - OMP_CLAUSE_PARALLEL  
 * - OMP_CLAUSE_SECTIONS
 * - OMP_CLAUSE_TASKGROUP
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_clauses.c -o test_program
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

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
            volatile int x = i + 1;
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

/* Function targeted with combination of clauses */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        volatile int x = i * 2;
    }
}

/* Function using taskgroup construct */
void func_taskgroup(int n) {
    int i;
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < n; i++) {
                #pragma omp task
                {
                    volatile int x = i;
                }
                
                /* Use taskgroup to wait for tasks */
                if (i % 10 == 0) {
                    #pragma omp taskgroup
                    {
                        #pragma omp task
                        {
                            volatile int y = i * 2;
                        }
                    }
                }
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare variant(func_for) match(construct={dispatch}, device={arch(any)})
#pragma omp declare variant(func_parallel) match(construct={dispatch}, device={arch(any)})
void func_dispatch(int n) {
    #pragma omp dispatch
    func_for(n);
}

/* Main function that calls all OpenMP functions */
int main(void) {
    int n = 100;
    
    /* Call functions to ensure they're not eliminated */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_combined(n);
    func_taskgroup(n);
    
    /* Try dispatch variant */
    #pragma omp target device(0)
    func_dispatch(n);
    
    /* Additional constructs to ensure rich OpenMP parse tree */
    #pragma omp target teams distribute parallel for map(tofrom:n)
    for (int i = 0; i < n; i++) {
        volatile int x = i * 3;
    }
    
    /* Nested parallel region with sections */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { volatile int a = 1; }
            #pragma omp section
            { volatile int b = 2; }
        }
    }
    
    return 0;
}
