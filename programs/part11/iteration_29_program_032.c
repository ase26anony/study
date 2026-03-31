/* test-omp-pretty-print.c
 * This program is designed to trigger coverage of specific OpenMP clause
 * pretty-printing code in GCC's tree-pretty-print.cc (lines 1434-1445).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-pretty-print.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Function targeted with 'for' clause in declare target directive */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some computation */
        printf("for iteration %d\n", i);
    }
}

/* Function targeted with 'parallel' clause in declare target directive */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel private(i)
    {
        #pragma omp for
        for (i = 0; i < n; i++) {
            printf("parallel iteration %d\n", i);
        }
    }
}

/* Function targeted with 'sections' clause in declare target directive */
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

/* Function using taskgroup construct - clause appears in pretty-printing */
void func_taskgroup(int n) {
    int i;
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < n; i++) {
                #pragma omp task
                {
                    printf("Task %d\n", i);
                }
            }
            #pragma omp taskgroup
            {
                /* Taskgroup region */
                printf("Inside taskgroup\n");
            }
        }
    }
}

/* Function using combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("Combined iteration %d\n", i);
    }
}

/* Function using dispatch directive with parallel clause */
void func_dispatch(void) {
    int device_id = 0;
    #pragma omp dispatch device(device_id) parallel
    {
        printf("Dispatch parallel region\n");
    }
}

int main(void) {
    int n = 5;
    
    /* Call all functions to ensure they're compiled and not eliminated */
    printf("Testing func_for:\n");
    func_for(n);
    
    printf("\nTesting func_parallel:\n");
    func_parallel(n);
    
    printf("\nTesting func_sections:\n");
    func_sections();
    
    printf("\nTesting func_taskgroup:\n");
    func_taskgroup(n);
    
    printf("\nTesting func_combined:\n");
    func_combined(n);
    
    printf("\nTesting func_dispatch:\n");
    func_dispatch();
    
    /* Also test declare target with variables */
    int target_var = 42;
    #pragma omp declare target to(target_var) for
    #pragma omp target map(tofrom: target_var)
    {
        target_var++;
    }
    printf("\nTarget variable value: %d\n", target_var);
    
    return 0;
}
