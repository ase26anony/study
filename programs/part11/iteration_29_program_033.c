/* test-omp-pretty-print.c
 * This program is designed to trigger the uncovered pretty-printer
 * logic for OpenMP clause names (for, parallel, sections, taskgroup)
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
        printf("for loop iteration %d\n", i);
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
        { printf("Section 1\n"); }
        #pragma omp section
        { printf("Section 2\n"); }
    }
}

/* Function targeted with both 'for' and 'parallel' clauses */
#pragma omp declare target to(func_mixed) for parallel
void func_mixed(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("Mixed: iteration %d, thread %d\n", i, omp_get_thread_num());
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
                { printf("Task 1\n"); }
                #pragma omp task
                { printf("Task 2\n"); }
            }
            printf("Taskgroup completed\n");
        }
    }
}

/* Function using dispatch construct with parallel clause */
void func_dispatch(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        printf("Dispatch parallel: %d\n", i);
    }
}

/* Main function that references all above functions */
int main(void) {
    int n = 10;
    
    /* Call functions to ensure they're not eliminated */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_mixed(n);
    func_taskgroup(n);
    func_dispatch(n);
    
    /* Additional OpenMP constructs for richer context */
    
    /* Nested parallel with sections */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { printf("Main sections A\n"); }
            #pragma omp section
            { printf("Main sections B\n"); }
        }
    }
    
    /* Task generating region with taskgroup */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (int i = 0; i < 5; i++) {
                    #pragma omp task
                    { printf("Task %d\n", i); }
                }
            }
        }
    }
    
    /* Combined parallel for with reduction */
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    printf("Sum = %d\n", sum);
    
    return 0;
}
