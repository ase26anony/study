/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, PARALLEL, SECTIONS, TASKGROUP pretty-printing */

#include <stdio.h>
#include <stdlib.h>

/* Function targeted with 'for' clause in declare target */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some computation */
        printf("For iteration %d\n", i);
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
            printf("Parallel iteration %d\n", i);
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
#pragma omp declare target to(func_taskgroup)
void func_taskgroup(int n) {
    int i;
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

/* Function using combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        printf("Combined iteration %d\n", i);
    }
}

/* Function using dispatch directive with parallel clause */
void func_dispatch(void) {
    int device_id = 0;
    #pragma omp dispatch device(device_id) parallel
    {
        printf("Dispatch parallel region\n");
    }
}

/* Main function that calls all test functions */
int main(void) {
    int n = 10;
    
    /* Call functions to ensure they're referenced */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_taskgroup(n);
    func_combined(n);
    func_dispatch();
    
    /* Additional constructs to ensure rich OpenMP tree */
    #pragma omp target teams distribute parallel for map(tofrom:n)
    for (int i = 0; i < n; i++) {
        n += i;
    }
    
    /* Nested parallel region with sections */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                #pragma omp task
                {
                    printf("Nested task\n");
                }
            }
            #pragma omp section
            {
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
    
    return 0;
}
