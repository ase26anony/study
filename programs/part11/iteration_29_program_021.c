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
            volatile int x = i * 2;
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

/* Function targeted with combined clauses */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        volatile int x = i * 3;
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
                    volatile int x = n;
                }
                #pragma omp task
                {
                    volatile int y = n * 2;
                }
            }
        }
    }
}

/* Function using dispatch directive with parallel clause */
#pragma omp declare variant(func_parallel) match(construct={dispatch}, device={arch(any)})
void func_dispatch(int n) {
    /* Fallback implementation */
    for (int i = 0; i < n; i++) {
        volatile int x = i;
    }
}

/* Main function that calls all test functions */
int main(void) {
    int n = 100;
    
    /* Call functions to ensure they're not eliminated */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_combined(n);
    func_taskgroup(n);
    func_dispatch(n);
    
    /* Additional test with declare target at file scope */
    int target_var = 42;
    #pragma omp declare target link(target_var)
    
    /* Use sections construct directly in main */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            target_var += 1;
        }
        #pragma omp section
        {
            target_var += 2;
        }
    }
    
    printf("Final value: %d\n", target_var);
    
    return 0;
}
