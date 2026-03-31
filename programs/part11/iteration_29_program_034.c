/* test_omp_clauses.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_clauses.c -o test_program
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
        printf("func_for: iteration %d\n", i);
    }
}

/* Function targeted with 'parallel' clause in declare target */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("func_parallel: iteration %d\n", i);
    }
}

/* Function targeted with 'sections' clause in declare target */
#pragma omp declare target to(func_sections) sections
void func_sections(void) {
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            printf("Section 1 executed by thread %d\n", omp_get_thread_num());
        }
        #pragma omp section
        {
            printf("Section 2 executed by thread %d\n", omp_get_thread_num());
        }
    }
}

/* Function targeted with both 'for' and 'parallel' clauses */
#pragma omp declare target to(func_mixed) for parallel
void func_mixed(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("func_mixed: iteration %d\n", i);
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
                    printf("Task %d executed by thread %d\n", 
                           i, omp_get_thread_num());
                }
            }
            #pragma omp taskgroup
            {
                printf("Taskgroup region\n");
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare variant(func_dispatch_variant) \
    match(construct={dispatch}, device={arch(x86_64)})
void func_dispatch_variant(int n) {
    printf("Dispatch variant called\n");
}

void func_dispatch(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        printf("Dispatch iteration %d\n", i);
    }
}

/* Main function that calls all test functions */
int main(void) {
    int n = 5;
    
    /* Call functions to ensure they're not eliminated */
    printf("Testing func_for:\n");
    func_for(n);
    
    printf("\nTesting func_parallel:\n");
    func_parallel(n);
    
    printf("\nTesting func_sections:\n");
    func_sections();
    
    printf("\nTesting func_mixed:\n");
    func_mixed(n);
    
    printf("\nTesting func_taskgroup:\n");
    func_taskgroup(n);
    
    printf("\nTesting func_dispatch:\n");
    func_dispatch(n);
    
    /* Additional complex OpenMP region with nested constructs */
    printf("\nTesting complex nested region:\n");
    #pragma omp target teams distribute parallel for map(tofrom:n)
    for (int i = 0; i < n; i++) {
        printf("Target region iteration %d\n", i);
    }
    
    return 0;
}
