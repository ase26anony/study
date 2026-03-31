/* test-omp-pretty-print.c
 * 
 * This program is designed to trigger the pretty-printing logic in GCC's
 * tree-pretty-print.cc for the uncovered OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP cases.
 * 
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-pretty-print.c -o test
 * This will generate a tree dump file (test-omp-pretty-print.c.*.omplower) that
 * should contain pretty-printed representations of the targeted clauses.
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
        printf("for: %d\n", i);
    }
}

/* Function targeted with 'declare target parallel' clause */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some work */
        printf("parallel: %d\n", i);
    }
}

/* Function targeted with 'declare target sections' clause */
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

/* Function targeted with combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel sections
void func_combined(void) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < 5; i++) {
            printf("Combined for: %d\n", i);
        }
        
        #pragma omp sections
        {
            #pragma omp section
            { printf("Combined section A\n"); }
            #pragma omp section
            { printf("Combined section B\n"); }
        }
    }
}

/* Function using taskgroup construct */
void func_taskgroup(void) {
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
#pragma omp declare variant(func_parallel) match(construct={dispatch}, device={arch(any)})
void func_dispatch_variant(int n) {
    /* Fallback implementation */
    for (int i = 0; i < n; i++) {
        printf("dispatch fallback: %d\n", i);
    }
}

/* Main function that calls all the above to ensure they're compiled */
int main(void) {
    int n = 10;
    
    /* Call functions to ensure they're not optimized away */
    #pragma omp target teams map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
    }
    
    func_sections();
    func_combined();
    func_taskgroup();
    func_dispatch_variant(5);
    
    /* Additional test: declare target at file scope with multiple clauses */
    int file_var = 42;
    #pragma omp declare target link(file_var)
    
    /* Use dispatch directive with parallel clause */
    #pragma omp dispatch parallel
    func_parallel(3);
    
    return 0;
}
