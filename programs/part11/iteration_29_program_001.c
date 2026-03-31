/* test-omp-pretty-print.c
 * This program is designed to trigger coverage of specific OpenMP clause
 * pretty-printing code in GCC's tree-pretty-print.cc (lines 1434-1445).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-pretty-print.c -o test
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
        /* Some computation */
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
            /* Some computation */
            int y = i + 1;
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
#pragma omp declare target to(func_combined) for parallel sections
void func_combined(int n) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            /* Loop body */
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
void func_dispatch(int n) {
    #pragma omp dispatch parallel
    for (int i = 0; i < n; i++) {
        printf("Dispatch iteration %d\n", i);
    }
}

/* Main function that calls all OpenMP functions */
int main(void) {
    int n = 100;
    
    /* Call functions to ensure they're not dead code */
    #pragma omp target teams map(tofrom: n)
    {
        func_for(n);
        func_parallel(n);
    }
    
    func_sections();
    func_combined(n);
    func_taskgroup(n);
    func_dispatch(n);
    
    /* Additional constructs to ensure rich OpenMP parse tree */
    
    /* Nested parallel with for clause context */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        /* Some computation */
    }
    
    /* Sections inside parallel region */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { /* section A */ }
            #pragma omp section
            { /* section B */ }
        }
    }
    
    /* Multiple taskgroups */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                { /* task 1 */ }
            }
            
            #pragma omp taskgroup
            {
                #pragma omp task
                { /* task 2 */ }
            }
        }
    }
    
    return 0;
}
