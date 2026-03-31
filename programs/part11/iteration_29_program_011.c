/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function targeted with 'for' clause */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("For clause test: %d\n", i);
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
            printf("Parallel clause test: %d\n", i);
        }
    }
}

/* Function targeted with 'sections' clause */
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

/* Function targeted with combined clauses */
#pragma omp declare target to(func_combined) for parallel sections
void func_combined(int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            printf("Combined: %d\n", i);
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

/* Function using taskgroup clause */
void func_taskgroup(int n) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    printf("Task 1\n");
                }
                #pragma omp task
                {
                    printf("Task 2\n");
                }
            }
        }
    }
}

/* Function using dispatch directive with parallel clause */
#pragma omp declare variant(func_dispatch_variant) match(construct={dispatch}, device={arch(x86_64)})
void func_dispatch_variant(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("Dispatch variant: %d\n", i);
    }
}

void func_dispatch(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        printf("Dispatch: %d\n", i);
    }
}

/* Main function that calls all test functions */
int main(void) {
    int n = 10;
    
    /* Call functions to ensure they're not optimized away */
    #pragma omp target teams map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
        func_sections();
        func_combined(n);
    }
    
    func_taskgroup(n);
    func_dispatch(n);
    func_dispatch_variant(n);
    
    /* Additional complex OpenMP region */
    #pragma omp target teams distribute parallel for map(tofrom: n)
    for (int i = 0; i < n; i++) {
        printf("Final: %d\n", i);
    }
    
    return 0;
}
