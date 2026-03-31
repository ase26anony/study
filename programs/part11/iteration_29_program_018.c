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
            for (i = 0; i < n; i++) {
                #pragma omp task
                {
                    printf("Task %d\n", i);
                }
            }
            #pragma omp taskgroup
            {
                /* Wait for all tasks */
                #pragma omp taskwait
            }
        }
    }
}

/* Function using multiple clauses in declare target */
#pragma omp declare target to(func_mixed) for parallel
void func_mixed(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("Mixed iteration %d\n", i);
    }
}

/* Function using dispatch directive with parallel clause */
void func_dispatch(void) {
    int device_id = 0;
    #pragma omp dispatch device(device_id) parallel
    {
        printf("Dispatch with parallel clause\n");
    }
}

/* Main function that calls all OpenMP functions */
int main(void) {
    int n = 10;
    
    /* Call functions to ensure they're not eliminated */
    #pragma omp target map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
        func_sections();
        func_taskgroup(n);
        func_mixed(n);
    }
    
    func_dispatch();
    
    /* Additional constructs to ensure rich OpenMP tree */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        printf("Teams distribute %d\n", i);
    }
    
    /* Nested parallel region with sections */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                printf("Nested section A\n");
            }
            #pragma omp section
            {
                printf("Nested section B\n");
            }
        }
    }
    
    /* Taskgroup in tasking context */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                printf("Task in taskgroup\n");
                
                #pragma omp task
                printf("Another task in taskgroup\n");
            }
        }
    }
    
    return 0;
}
