/* test_omp_clauses.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_clauses.c -o test_program
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
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        printf("Parallel thread %d\n", tid);
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
#pragma omp declare target to(func_taskgroup)
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
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("Combined iteration %d\n", i);
    }
}

/* Function using dispatch directive with parallel clause */
void func_dispatch(void) {
    #pragma omp dispatch parallel
    for (int i = 0; i < 10; i++) {
        printf("Dispatch iteration %d\n", i);
    }
}

/* Main function that calls all test functions */
int main(void) {
    int n = 5;
    
    /* Call functions to ensure they're not eliminated */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_taskgroup(n);
    func_combined(n);
    func_dispatch();
    
    /* Additional test with declare target at file scope */
    int target_var = 42;
    #pragma omp declare target to(target_var)
    
    /* Use target_var in parallel region */
    #pragma omp target map(tofrom: target_var)
    {
        target_var++;
    }
    
    printf("Final target_var: %d\n", target_var);
    
    return 0;
}
