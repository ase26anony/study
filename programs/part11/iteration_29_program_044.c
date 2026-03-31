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
        // Some computation
        printf("for iteration %d\n", i);
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
            printf("parallel iteration %d\n", i);
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

/* Function with taskgroup construct */
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

/* Function targeted with combined clauses */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("combined iteration %d\n", i);
    }
}

/* Dispatch directive with parallel clause */
#pragma omp dispatch device(0) parallel
void func_dispatch(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("dispatch iteration %d\n", i);
    }
}

/* Main function to call all targeted functions */
int main(void) {
    int n = 10;
    
    #pragma omp target map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
        func_sections();
        func_combined(n);
    }
    
    func_taskgroup(n);
    func_dispatch(n);
    
    return 0;
}
