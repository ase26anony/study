/* test-omp-pretty-print.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * tree-pretty-print.cc (lines 1434-1445) that handle the pretty-printing
 * of OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, and
 * OMP_CLAUSE_TASKGROUP clause names.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-pretty-print.c -o test
 * This will generate a tree dump file (test.c.omplower) that should contain
 * the pretty-printed clause names when the compiler processes the OpenMP constructs.
 */

#include <stdio.h>
#include <stdlib.h>

/* Function targeted with 'declare target for' clause */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some work */
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

/* Function containing a taskgroup construct */
void func_taskgroup(int n) {
    int i;
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < n; i++) {
                #pragma omp task
                {
                    /* Some task work */
                    printf("Task %d\n", i);
                }
            }
            #pragma omp taskgroup
            {
                /* taskgroup construct - its name may appear in dumps */
                #pragma omp task
                {
                    printf("Task within taskgroup\n");
                }
            }
        }
    }
}

/* Function using 'declare target' with combined clauses */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("Combined function iteration %d\n", i);
    }
}

/* Function using 'omp dispatch' with parallel clause */
void func_dispatch(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        printf("Dispatch parallel iteration %d\n", i);
    }
}

/* Main function that calls all the above to ensure they're compiled */
int main(void) {
    int n = 10;
    
    /* Call each function to prevent dead-code elimination */
    #pragma omp target map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
        func_sections();
        func_combined(n);
    }
    
    func_taskgroup(n);
    func_dispatch(n);
    
    return 0;
}
