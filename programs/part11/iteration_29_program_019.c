/* test_omp_coverage.c
 * This program is designed to trigger coverage of specific OpenMP clause
 * pretty-printing code in GCC's tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_coverage.c -o test_omp_coverage
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
        printf("for iteration %d\n", i);
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
            printf("parallel iteration %d\n", i);
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
            printf("Section 1\n");
        }
        #pragma omp section
        {
            printf("Section 2\n");
        }
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
                    {
                        printf("Task %d\n", i);
                    }
                }
            }
        }
    }
}

/* Function using combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("combined iteration %d\n", i);
    }
}

/* Function using dispatch with parallel clause */
#pragma omp declare variant(func_dispatch_variant) match(construct={dispatch}, device={arch(any)})
void func_dispatch_base(int n) {
    printf("Base implementation\n");
}

void func_dispatch_variant(int n) {
    #pragma omp dispatch parallel
    {
        printf("Dispatch variant with parallel clause\n");
    }
}

/* Main function that calls all test functions */
int main(void) {
    int n = 10;
    
    /* Call functions to ensure they're referenced and compiled */
    #pragma omp target teams map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
    }
    
    func_sections();
    func_taskgroup(n);
    func_combined(n);
    func_dispatch_base(n);
    func_dispatch_variant(n);
    
    /* Additional test with declare target at file scope */
    int file_scope_var = 42;
    #pragma omp declare target link(file_scope_var)
    
    /* Nested constructs for richer context */
    #pragma omp target teams distribute parallel for map(tofrom: n)
    for (int i = 0; i < n; i++) {
        printf("Nested construct iteration %d\n", i);
    }
    
    return 0;
}
