/* test-omp-pretty-print.c
 * This program is designed to trigger coverage of specific OpenMP clause
 * pretty-printing code in GCC's tree-pretty-print.cc (lines 1434-1445).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-pretty-print.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function targeted with 'for' clause in declare target directive */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some computation */
        int x = i * i;
    }
}

/* Function targeted with 'parallel' clause in declare target directive */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel
    {
        #pragma omp for
        for (i = 0; i < n; i++) {
            /* Some computation */
            int y = i + 1;
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

/* Function targeted with combination of clauses */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        /* Combined computation */
        int z = i % 10;
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

/* Function using dispatch directive with parallel clause */
#pragma omp declare variant(func_dispatch_variant) match(construct={dispatch}, device={arch(x86_64)})
void func_dispatch_base(int n) {
    printf("Base implementation\n");
}

void func_dispatch_variant(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        printf("Dispatch variant: %d\n", i);
    }
}

/* Main function that calls all OpenMP functions */
int main(void) {
    int n = 100;
    
    /* Ensure all functions are called to prevent dead code elimination */
    #pragma omp target map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
        func_sections();
        func_combined(n);
    }
    
    func_taskgroup(n);
    func_dispatch_base(n);
    func_dispatch_variant(n);
    
    /* Additional test: nested sections inside parallel region */
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
    
    return 0;
}
