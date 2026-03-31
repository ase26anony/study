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
        int x = i * i;
    }
}

/* Function targeted with 'parallel' clause in declare target */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel private(i)
    {
        #pragma omp for
        for (i = 0; i < n; i++) {
            /* Some work */
            int y = i % 10;
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

/* Function targeted with combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel sections
void func_combined(int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            /* Work */
        }
        
        #pragma omp sections
        {
            #pragma omp section
            { /* empty */ }
            #pragma omp section
            { /* empty */ }
        }
    }
}

/* Function using taskgroup construct */
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

/* Function using dispatch construct with parallel clause */
void func_dispatch(void) {
    int device_id = 0;
    #pragma omp dispatch device(device_id) parallel
    {
        printf("Dispatch parallel region\n");
    }
}

/* Main function that references all above functions to ensure they're compiled */
int main(void) {
    int n = 100;
    
    /* Call functions to ensure they're not eliminated */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_combined(n);
    func_taskgroup(n);
    func_dispatch();
    
    /* Additional parallel region with sections */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                printf("Main section A\n");
            }
            #pragma omp section
            {
                printf("Main section B\n");
            }
        }
    }
    
    /* Additional for loop in parallel region */
    #pragma omp parallel for
    for (int i = 0; i < 10; i++) {
        printf("Iteration %d\n", i);
    }
    
    return 0;
}
