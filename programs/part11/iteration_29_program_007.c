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

/* Function using dispatch directive with parallel clause */
#pragma omp declare target to(func_dispatch)
void func_dispatch(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        printf("Dispatch iteration %d\n", i);
    }
}

/* Function with combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("Combined iteration %d\n", i);
    }
}

/* Main function that references all OpenMP functions to ensure they're compiled */
int main(void) {
    int n = 5;
    
    printf("Testing OpenMP clause pretty-printing coverage...\n");
    
    /* Call each function to ensure they're not eliminated */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_taskgroup(n);
    func_dispatch(n);
    func_combined(n);
    
    /* Additional OpenMP constructs for context */
    
    /* Nested parallel region with sections */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { printf("Nested section A\n"); }
            #pragma omp section
            { printf("Nested section B\n"); }
        }
    }
    
    /* Taskgroup in a task-generating region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                { printf("Task in taskgroup\n"); }
            }
        }
    }
    
    /* declare target with device_type clause (additional context) */
    #pragma omp declare target device_type(host) to(host_func)
    void host_func(void) {
        printf("Host function\n");
    }
    
    host_func();
    
    return 0;
}
