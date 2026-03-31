/* test-omp-pretty-print.c
 * This program is designed to trigger coverage of specific OpenMP clause
 * pretty-printing code in GCC's tree-pretty-print.cc (lines 1434-1445).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-pretty-print.c -o test
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
        int x = i * i;
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
            /* Some computation */
            int y = i * 2;
        }
    }
}

/* Function targeted with 'sections' clause in declare target */
#pragma omp declare target to(func_sections) sections
void func_sections(void) {
    #pragma omp parallel sections
    {
        #pragma omp section
        { printf("Section 1\n"); }
        #pragma omp section
        { printf("Section 2\n"); }
    }
}

/* Function targeted with both 'for' and 'parallel' clauses */
#pragma omp declare target to(func_mixed) for parallel
void func_mixed(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        /* Mixed computation */
        int z = i % 10;
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
                { printf("Task 1\n"); }
                #pragma omp task
                { printf("Task 2\n"); }
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare variant(func_parallel) match(construct={dispatch}, device={arch(x86_64)})
void func_dispatch(int n) {
    /* Fallback implementation */
    for (int i = 0; i < n; i++) {
        /* Default computation */
    }
}

int main(void) {
    int n = 100;
    
    /* Call all functions to ensure they're compiled and used */
    #pragma omp target data map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
        func_sections();
        func_mixed(n);
    }
    
    func_taskgroup(n);
    func_dispatch(n);
    
    /* Additional structured blocks with sections clause */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { printf("Main section A\n"); }
            #pragma omp section
            { printf("Main section B\n"); }
        }
    }
    
    /* Nested parallel region */
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < 10; i++) {
            #pragma omp taskgroup
            {
                #pragma omp task
                { printf("Nested task %d\n", i); }
            }
        }
    }
    
    return 0;
}
