/* test_omp_clauses.c */
/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_clauses.c -o test_program */

#include <stdio.h>
#include <stdlib.h>

/* Function targeted with 'for' clause */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("for clause test: %d\n", i);
    }
}

/* Function targeted with 'parallel' clause */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel
    {
        #pragma omp for
        for (i = 0; i < n; i++) {
            printf("parallel clause test: %d\n", i);
        }
    }
}

/* Function targeted with 'sections' clause */
#pragma omp declare target to(func_sections) sections
void func_sections(void) {
    #pragma omp parallel sections
    {
        #pragma omp section
        printf("Section 1\n");
        #pragma omp section
        printf("Section 2\n");
    }
}

/* Function targeted with combined clauses */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("combined clauses test: %d\n", i);
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
                #pragma omp task
                {
                    for (i = 0; i < n; i++) {
                        printf("taskgroup test: %d\n", i);
                    }
                }
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare variant(func_parallel) match(construct={dispatch}, device={arch(any)})
void func_dispatch(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        printf("dispatch with parallel clause: %d\n", i);
    }
}

/* Main function to ensure all functions are referenced */
int main(void) {
    int n = 5;
    
    /* Call all functions to ensure they're not eliminated */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_combined(n);
    func_taskgroup(n);
    func_dispatch(n);
    
    /* Additional constructs to ensure rich OpenMP tree */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        printf("target teams: %d\n", i);
    }
    
    /* Nested sections inside parallel region */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            printf("Nested section A\n");
            #pragma omp section
            printf("Nested section B\n");
        }
    }
    
    return 0;
}
