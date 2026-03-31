/* test-omp-pretty-print.c
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clause names (for, parallel, sections, taskgroup)
 * in GCC's tree-pretty-print.cc when compiled with -fdump-tree-* flags.
 */

#include <stdio.h>
#include <stdlib.h>

/* Function targeted with 'declare target for' clause */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some computation */
        printf("for loop iteration %d\n", i);
    }
}

/* Function targeted with 'declare target parallel' clause */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel private(i)
    {
        #pragma omp for
        for (i = 0; i < n; i++) {
            printf("parallel region iteration %d\n", i);
        }
    }
}

/* Function targeted with 'declare target sections' clause */
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
            for (i = 0; i < n; i++) {
                #pragma omp task
                {
                    printf("Task %d\n", i);
                }
            }
            #pragma omp taskgroup
            {
                printf("Inside taskgroup\n");
            }
        }
    }
}

/* Combined clause usage in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("Combined function iteration %d\n", i);
    }
}

/* Test dispatch construct with parallel clause */
void func_dispatch(void) {
    int device_id = 0;
    #pragma omp dispatch device(device_id) parallel
    {
        printf("Dispatch with parallel clause\n");
    }
}

int main(void) {
    int n = 5;
    
    /* Call all functions to ensure they're not eliminated */
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
    
    /* Additional OpenMP constructs for context */
    printf("\nTesting nested constructs:\n");
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        printf("Target teams iteration %d\n", i);
    }
    
    return 0;
}
