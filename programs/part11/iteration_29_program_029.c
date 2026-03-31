/* test-omp-pretty-print.c
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clauses (for, parallel, sections, taskgroup) in
 * GCC's tree-pretty-print.cc when compiled with -fdump-tree-* flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function targeted with 'declare target for' clause */
#pragma omp declare target to(func_for) device_type(host) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some computation */
        printf("for loop iteration %d\n", i);
    }
}

/* Function targeted with 'declare target parallel' clause */
#pragma omp declare target to(func_parallel) device_type(host) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("parallel iteration %d\n", i);
    }
}

/* Function targeted with 'declare target sections' clause */
#pragma omp declare target to(func_sections) device_type(host) sections
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
#pragma omp declare target to(func_taskgroup) device_type(host)
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
                /* Taskgroup ensures all tasks complete */
                printf("Taskgroup region\n");
            }
        }
    }
}

/* Function using combined clauses in declare target */
#pragma omp declare target to(func_combined) device_type(host) for parallel
void func_combined(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("Combined iteration %d\n", i);
    }
}

/* Function using dispatch construct with parallel clause */
void func_dispatch(int n) {
    #pragma omp dispatch parallel for
    for (int i = 0; i < n; i++) {
        printf("Dispatch iteration %d\n", i);
    }
}

/* Main function to ensure all functions are referenced */
int main(int argc, char **argv) {
    int n = 5;
    
    /* Call functions to ensure they're not dead-code eliminated */
    #pragma omp target map(tofrom: n) if(target:0)
    {
        func_for(n);
        func_parallel(n);
        func_sections();
        func_taskgroup(n);
        func_combined(n);
    }
    
    func_dispatch(n);
    
    /* Additional parallel region with sections */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { printf("Main section A\n"); }
            #pragma omp section
            { printf("Main section B\n"); }
        }
    }
    
    /* Additional taskgroup */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                printf("Task in main taskgroup\n");
            }
        }
    }
    
    return 0;
}
