/* test-omp-pretty-print.c
 * 
 * This program is designed to trigger the uncovered pretty-printing
 * logic for OpenMP clauses: for, parallel, sections, and taskgroup.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-pretty-print.c -o test
 * The tree dump will contain pretty-printed representations of these clauses.
 */

#include <stdio.h>
#include <stdlib.h>

/* Function targeted with 'declare target for' clause */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some work */
    }
}

/* Function targeted with 'declare target parallel' clause */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        /* Some parallel work */
    }
}

/* Function targeted with 'declare target sections' clause */
#pragma omp declare target to(func_sections) sections
void func_sections(void) {
    #pragma omp parallel sections
    {
        #pragma omp section
        { /* Section 1 work */ }
        #pragma omp section
        { /* Section 2 work */ }
    }
}

/* Function containing a taskgroup construct */
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
                    { /* Some task work */ }
                }
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare variant(func_parallel) match(construct={dispatch}, device={arch(any)})
void func_dispatch(int n) {
    /* This variant will be associated with a dispatch directive
       that may include the 'parallel' clause in its internal representation */
}

/* Combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel sections
void func_combined(void) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < 10; i++) { }
        #pragma omp sections
        {
            #pragma omp section
            { }
            #pragma omp section
            { }
        }
    }
}

int main(void) {
    int n = 100;

    /* Ensure all functions are called to prevent dead-code elimination */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_taskgroup(n);
    func_combined();

    /* Also call the dispatch variant */
    #pragma omp dispatch
    func_dispatch(n);

    printf("OpenMP clause pretty-print test completed.\n");
    return 0;
}
