/* test_omp_clauses.c
 * Designed to trigger pretty-printing of OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_clauses.c -o test
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
        // Some computation
        int x = i * i;
    }
}

/* Function targeted with 'parallel' clause in declare target */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel
    {
        #pragma omp for
        for (i = 0; i < n; i++) {
            // Parallel computation
            int y = i * 2;
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

/* Function targeted with combination of clauses */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        // Combined computation
        int z = i * 3;
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
#pragma omp declare variant(func_dispatch) match(construct={dispatch}, device={arch(any)})
void func_dispatch(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("Dispatch %d\n", i);
    }
}

/* Main function to ensure all functions are referenced */
int main(void) {
    int n = 100;
    
    // Call functions to ensure they're not eliminated
    func_for(n);
    func_parallel(n);
    func_sections();
    func_combined(n);
    func_taskgroup(n);
    
    // Use dispatch
    #pragma omp dispatch
    func_dispatch(5);
    
    // Additional constructs to ensure rich OpenMP parse tree
    #pragma omp target teams distribute parallel for map(tofrom:n)
    for (int i = 0; i < n; i++) {
        // Target region
    }
    
    // Nested parallel region with sections
    #pragma omp parallel
    {
        #pragma omp master
        {
            #pragma omp taskloop
            for (int i = 0; i < 10; i++) {
                printf("Taskloop iteration %d\n", i);
            }
        }
    }
    
    return 0;
}
