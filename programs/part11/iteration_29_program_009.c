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
                #pragma omp task
                {
                    printf("Task in taskgroup\n");
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
        printf("Combined iteration %d\n", i);
    }
}

/* Function using dispatch construct with parallel clause */
void func_dispatch(void) {
    int device_id = 0;
    #pragma omp dispatch device(device_id) parallel
    {
        printf("Dispatch with parallel clause\n");
    }
}

/* Main function that calls all test functions */
int main(void) {
    int n = 5;
    
    /* Ensure all functions are called to prevent dead code elimination */
    #pragma omp target map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
        func_sections();
        func_taskgroup(n);
        func_combined(n);
    }
    
    func_dispatch();
    
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
    
    return 0;
}
