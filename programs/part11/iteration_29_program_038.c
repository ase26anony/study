/* test-omp-pretty-print.c
 * This program is designed to trigger coverage of specific OpenMP clause
 * pretty-printing code in GCC's tree-pretty-print.cc (lines 1434-1445).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-pretty-print.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

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
        printf("parallel iteration %d\n", i);
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
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            printf("Mixed: %d\n", i);
        }
    }
}

/* Function containing a taskgroup construct */
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
        }
    }
}

/* Function using omp dispatch with parallel clause */
void func_dispatch(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        printf("Dispatch parallel: %d\n", i);
    }
}

/* Main function calls all to ensure they're compiled and used */
int main(void) {
    int n = 5;
    
    #pragma omp target map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
        func_sections();
        func_mixed(n);
    }
    
    func_taskgroup(n);
    func_dispatch(n);
    
    return 0;
}
