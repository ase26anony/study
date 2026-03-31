/* test_omp_clauses.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_clauses.c -o test_program
 */

#include <stdio.h>
#include <stdlib.h>

/* Function targeted with 'for' clause */
#pragma omp declare target to(func_for) for
void func_for(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        /* Some computation */
        printf("for iteration %d\n", i);
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
            printf("parallel iteration %d\n", i);
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

/* Function targeted with both 'for' and 'parallel' clauses */
#pragma omp declare target to(func_mixed) for parallel
void func_mixed(int n) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        printf("mixed iteration %d\n", i);
    }
}

/* Function containing taskgroup construct */
void func_taskgroup(int n) {
    int i;
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < n; i++) {
                #pragma omp task
                {
                    printf("Task %d\n", i);
                }
            }
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

/* Function using dispatch directive with parallel clause */
#pragma omp declare target
void func_dispatch(int n) {
    int i;
    #pragma omp dispatch parallel for
    for (i = 0; i < n; i++) {
        printf("dispatch iteration %d\n", i);
    }
}
#pragma omp end declare target

/* Main function to ensure all functions are referenced */
int main(void) {
    int n = 10;
    
    /* Call all functions to ensure they're compiled */
    func_for(n);
    func_parallel(n);
    func_sections();
    func_mixed(n);
    func_taskgroup(n);
    func_dispatch(n);
    
    /* Additional test with declare target at file scope */
    int target_var = 42;
    #pragma omp declare target to(target_var)
    
    /* Use target construct to ensure declare target is processed */
    #pragma omp target map(tofrom: target_var)
    {
        target_var++;
    }
    
    printf("Final value: %d\n", target_var);
    
    return 0;
}
