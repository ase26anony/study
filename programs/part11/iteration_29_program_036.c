/* test_omp_clauses.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_clauses.c -o test_omp_clauses
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
        printf("for iteration %d\n", i);
    }
}

/* Function targeted with 'parallel' clause in declare target */
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
#pragma omp declare target
void func_taskgroup(int n) {
    int i;
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < n; i++) {
                #pragma omp task
                {
                    /* Use taskgroup to wait for tasks */
                    #pragma omp taskgroup
                    {
                        printf("Task %d\n", i);
                    }
                }
            }
        }
    }
}
#pragma omp end declare target

/* Function using dispatch directive with parallel clause */
void func_dispatch(void) {
    int device_id = 0;
    #pragma omp dispatch device(device_id) parallel
    for (int i = 0; i < 5; i++) {
        printf("Dispatch iteration %d\n", i);
    }
}

/* Function with combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel sections
void func_combined(void) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < 3; i++) {
            printf("Combined for %d\n", i);
        }
        
        #pragma omp sections
        {
            #pragma omp section
            printf("Combined section A\n");
            #pragma omp section
            printf("Combined section B\n");
        }
    }
}

/* Main function that calls all test functions */
int main(void) {
    int n = 10;
    
    printf("Testing OpenMP clause pretty-printing...\n");
    
    /* Call functions to ensure they're compiled and used */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_taskgroup(5);
    func_dispatch();
    func_combined();
    
    /* Additional structured blocks for context */
    #pragma omp parallel
    {
        #pragma omp master
        {
            printf("Master thread\n");
        }
        
        #pragma omp barrier
        
        #pragma omp for
        for (int i = 0; i < n; i++) {
            #pragma omp taskgroup
            {
                printf("Nested taskgroup %d\n", i);
            }
        }
    }
    
    /* Test declare target with variables */
    int target_var = 42;
    #pragma omp declare target to(target_var)
    
    #pragma omp target map(tofrom: target_var)
    {
        target_var *= 2;
    }
    
    printf("Final target_var: %d\n", target_var);
    
    return 0;
}
