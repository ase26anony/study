/* test-omp-pretty-print.c
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clause names (for, parallel, sections, taskgroup)
 * in GCC's tree-pretty-print.cc when compiled with -fdump-tree-* flags.
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
        printf("for loop iteration %d\n", i);
    }
}

/* Function targeted with 'parallel' clause in declare target */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some computation */
        printf("parallel iteration %d\n", i);
    }
}

/* Function targeted with 'sections' clause in declare target */
#pragma omp declare target to(func_sections) sections
void func_sections(void) {
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            printf("Section 1\n");
        }
        #pragma omp section
        {
            printf("Section 2\n");
        }
    }
}

/* Function targeted with combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("combined iteration %d\n", i);
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
                    /* Task work */
                    printf("Task %d\n", i);
                    
                    /* Nested taskgroup */
                    #pragma omp taskgroup
                    {
                        #pragma omp task
                        { printf("  Nested task in taskgroup\n"); }
                    }
                }
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare variant(func_parallel) match(construct={dispatch}, device={arch(x86_64)})
void func_dispatch(int n) {
    printf("Dispatch variant called\n");
}

/* Main function that references all above functions */
int main(void) {
    int n = 5;
    
    /* Call functions to ensure they're not eliminated */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_combined(n);
    func_taskgroup(n);
    
    /* Try dispatch */
    #pragma omp dispatch
    func_dispatch(n);
    
    /* Additional constructs to provide richer context */
    
    /* declare target with multiple clauses on variables */
    int var_for, var_parallel, var_sections;
    #pragma omp declare target to(var_for) for
    #pragma omp declare target to(var_parallel) parallel
    #pragma omp declare target to(var_sections) sections
    
    /* Use the variables */
    #pragma omp target map(tofrom: var_for, var_parallel, var_sections)
    {
        var_for = 1;
        var_parallel = 2;
        var_sections = 3;
    }
    
    /* Nested parallel region with sections */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { printf("Nested section A\n"); }
            #pragma omp section
            { printf("Nested section B\n"); }
        }
    }
    
    /* Parallel for with taskgroup inside */
    #pragma omp parallel for
    for (int i = 0; i < 3; i++) {
        #pragma omp taskgroup
        {
            #pragma omp task
            { printf("Task in parallel for %d\n", i); }
        }
    }
    
    return 0;
}
