/* test-omp-pretty-print.c
 * This program is designed to trigger coverage of specific OMP clause
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
        printf("for: %d\n", i);
    }
}

/* Function targeted with 'parallel' clause in declare target */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("parallel: %d\n", i);
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

/* Function using taskgroup construct */
void func_taskgroup(int n) {
    int i;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (i = 0; i < n; i++) {
                    #pragma omp task
                    { printf("Task %d\n", i); }
                }
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare variant(func_parallel) match(construct={dispatch}, device={arch(any)})
void func_dispatch_parallel(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        printf("dispatch parallel: %d\n", i);
    }
}

/* Combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("combined: %d\n", i);
    }
}

int main(void) {
    int n = 5;
    
    /* Call all functions to ensure they're not eliminated */
    #pragma omp target teams map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
    }
    
    func_sections();
    func_taskgroup(n);
    func_dispatch_parallel(n);
    func_combined(n);
    
    return 0;
}
