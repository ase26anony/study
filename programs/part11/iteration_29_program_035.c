/* test_omp_clauses.c
 * Designed to trigger pretty-printing of OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_clauses.c -o test
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

/* Function targeted with both 'for' and 'parallel' clauses */
#pragma omp declare target to(func_mixed) for parallel
void func_mixed(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        volatile int x = i * 2;
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
                {
                    volatile int x = 1;
                }
                #pragma omp task
                {
                    volatile int y = 2;
                }
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
void func_dispatch(void) {
    #pragma omp dispatch parallel
    for (int i = 0; i < 10; i++) {
        volatile int x = i;
    }
}

/* Variable targeted with sections clause */
int target_var;
#pragma omp declare target to(target_var) sections

int main(void) {
    int n = 100;
    
    /* Call all functions to ensure they're not eliminated */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_mixed(n);
    func_taskgroup(n);
    func_dispatch();
    
    /* Use the targeted variable */
    #pragma omp target map(tofrom: target_var)
    {
        target_var = 42;
    }
    
    printf("All OpenMP functions executed\n");
    
    /* Additional parallel region with sections */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { volatile int x = 1; }
            #pragma omp section
            { volatile int y = 2; }
        }
    }
    
    return 0;
}
