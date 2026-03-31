/* test_omp_clauses.c */
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
            // Some work
            int y = i % 10;
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

/* Function targeted with combined clauses */
#pragma omp declare target to(func_combined) for parallel sections
void func_combined(int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            // Work
        }
        
        #pragma omp sections
        {
            #pragma omp section
            { /* empty */ }
            #pragma omp section
            { /* empty */ }
        }
    }
}

/* Function using taskgroup construct */
#pragma omp declare target
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
#pragma omp end declare target

/* Test dispatch construct with parallel clause */
void test_dispatch(void) {
    int device_id = 0;
    #pragma omp dispatch device(device_id) parallel
    for (int i = 0; i < 10; i++) {
        printf("Dispatch iteration %d\n", i);
    }
}

int main(void) {
    int n = 100;
    
    // Call all functions to ensure they're referenced
    func_for(n);
    func_parallel(n);
    func_sections();
    func_combined(n);
    func_taskgroup(n);
    test_dispatch();
    
    // Additional taskgroup usage
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                printf("Main task A\n");
                #pragma omp task
                printf("Main task B\n");
            }
        }
    }
    
    return 0;
}
