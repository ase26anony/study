/* test-omp-clauses.c
 * This program is designed to trigger coverage of specific OpenMP clause
 * pretty-printing code in GCC's tree-pretty-print.cc (lines 1434-1445).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-clauses.c -o test
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
        printf("for loop iteration %d\n", i);
    }
}

/* Function targeted with 'parallel' clause in declare target */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some computation */
        printf("parallel iteration %d\n", i);
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

/* Function using combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("Combined iteration %d\n", i);
    }
}

/* Function using dispatch directive with parallel clause */
void func_dispatch(void) {
    int device_id = omp_get_default_device();
    #pragma omp dispatch device(device_id) parallel
    {
        printf("Dispatch parallel region\n");
    }
}

/* Main function that calls all test functions */
int main(void) {
    int n = 5;
    
    /* Call functions to ensure they're compiled and used */
    #pragma omp target map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
        func_sections();
        func_combined(n);
    }
    
    func_taskgroup(n);
    func_dispatch();
    
    /* Additional constructs to ensure rich OpenMP parse tree */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        printf("Teams distribute parallel for %d\n", i);
    }
    
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < n; i++) {
        printf("SIMD loop %d\n", i);
    }
    
    return 0;
}
