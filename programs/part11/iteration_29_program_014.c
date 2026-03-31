/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
                         OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function targeted with 'for' clause */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        // Some computation
        int x = i * i;
    }
}

/* Function targeted with 'parallel' clause */
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

/* Function targeted with 'sections' clause */
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
                        int z = i * 3;
                    }
                }
            }
        }
    }
}

/* Combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        int w = i * 4;
    }
}

/* Using dispatch directive with parallel clause */
#pragma omp declare variant(func_parallel) match(construct={dispatch}, device={arch(x86_64)})
void func_dispatch_variant(int n) {
    // Fallback implementation
    for (int i = 0; i < n; i++) {
        int v = i * 5;
    }
}

/* Main function that calls all to ensure they're compiled */
int main(void) {
    int n = 100;
    
    // Call functions to ensure they're referenced
    func_for(n);
    func_parallel(n);
    func_sections();
    func_taskgroup(n);
    func_combined(n);
    func_dispatch_variant(n);
    
    // Additional constructs to enrich the OpenMP parse tree
    
    // Nested parallel with sections
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
    
    // Taskgroup in main
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
    
    // Dispatch construct
    #pragma omp dispatch
    for (int i = 0; i < 10; i++) {
        printf("Dispatch iteration %d\n", i);
    }
    
    return 0;
}
