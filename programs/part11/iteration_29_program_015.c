/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, PARALLEL, SECTIONS, TASKGROUP pretty-printing */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function targeted with 'for' clause in declare target */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("For clause test: %d\n", i);
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
            printf("Parallel clause test: %d (thread %d)\n", 
                   i, omp_get_thread_num());
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
            printf("Sections clause test: section 1\n");
        }
        #pragma omp section
        {
            printf("Sections clause test: section 2\n");
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
                        printf("Taskgroup clause test: task %d\n", i);
                    }
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
        printf("Combined clause test: %d\n", i);
    }
}

/* Function using dispatch directive with parallel clause */
void func_dispatch(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        printf("Dispatch with parallel clause: %d\n", i);
    }
}

/* Main function that calls all test functions */
int main(void) {
    int n = 10;
    
    printf("Testing OpenMP clause pretty-printing coverage...\n");
    
    /* Call functions to ensure they're referenced and compiled */
    #pragma omp target teams map(tofrom: n) if(0)  /* if(0) ensures host execution */
    {
        func_for(n);
        func_parallel(n);
        func_sections();
        func_taskgroup(n);
        func_combined(n);
    }
    
    func_dispatch(n);
    
    /* Additional nested constructs for richer context */
    #pragma omp parallel
    {
        #pragma omp master
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                func_for(5);
                
                #pragma omp task
                func_sections();
            }
        }
        
        #pragma omp sections
        {
            #pragma omp section
            {
                #pragma omp parallel for
                for (int i = 0; i < 3; i++) {
                    printf("Nested section/for: %d\n", i);
                }
            }
            #pragma omp section
            {
                printf("Second nested section\n");
            }
        }
    }
    
    printf("Test completed.\n");
    return 0;
}
