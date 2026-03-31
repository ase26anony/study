/* test-omp-pretty-print.c
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clauses (for, parallel, sections, taskgroup) in
 * GCC's tree-pretty-print.cc when compiled with -fdump-tree-* flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function targeted with 'declare target for' clause */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some computation */
        int x = i * i;
    }
}

/* Function targeted with 'declare target parallel' clause */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel
    {
        #pragma omp for
        for (i = 0; i < n; i++) {
            /* Some computation */
            int y = i + 1;
        }
    }
}

/* Function targeted with 'declare target sections' clause */
#pragma omp declare target to(func_sections) sections
void func_sections(int n) {
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            int a = n * 2;
        }
        #pragma omp section
        {
            int b = n / 2;
        }
    }
}

/* Function using taskgroup construct */
#pragma omp declare target to(func_taskgroup)
void func_taskgroup(int n) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            int i;
            for (i = 0; i < n; i++) {
                #pragma omp task
                {
                    /* Some task work */
                    int val = i * 3;
                }
            }
            #pragma omp taskgroup
            {
                /* Explicit taskgroup region */
                #pragma omp task
                {
                    int final = n * n;
                }
            }
        }
    }
}

/* Combined clause usage in declare target */
#pragma omp declare target to(func_combined) for parallel sections
void func_combined(int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            /* Loop work */
        }
        
        #pragma omp sections
        {
            #pragma omp section
            { int x = 1; }
            #pragma omp section
            { int y = 2; }
        }
    }
}

/* Use dispatch construct with parallel clause */
void func_dispatch(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        /* Vectorized/SIMD loop */
    }
}

/* Main function to ensure all functions are referenced */
int main(int argc, char **argv) {
    int n = 100;
    
    /* Call each function to prevent dead code elimination */
    func_for(n);
    func_parallel(n);
    func_sections(n);
    func_taskgroup(n);
    func_combined(n);
    func_dispatch(n);
    
    /* Also use some runtime OpenMP */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        printf("Thread %d executed\n", tid);
    }
    
    return 0;
}
