/* test_omp_clauses.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_clauses.c -o test_program
 */

/* Function targeted with 'for' clause in declare target */
#pragma omp declare target to(func_for) for
void func_for(void) {
    int i;
    #pragma omp for
    for (i = 0; i < 10; i++) {
        /* do work */
    }
}

/* Function targeted with 'parallel' clause in declare target */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(void) {
    #pragma omp parallel
    {
        /* parallel region */
    }
}

/* Function targeted with 'sections' clause in declare target */
#pragma omp declare target to(func_sections) sections
void func_sections(void) {
    #pragma omp sections
    {
        #pragma omp section
        { /* section 1 */ }
        #pragma omp section
        { /* section 2 */ }
    }
}

/* Function using taskgroup construct */
#pragma omp declare target to(func_taskgroup)
void func_taskgroup(void) {
    #pragma omp taskgroup
    {
        #pragma omp task
        { /* task 1 */ }
        #pragma omp task
        { /* task 2 */ }
    }
}

/* Function using combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel sections
void func_combined(void) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < 5; i++) {
            /* loop work */
        }
        
        #pragma omp sections
        {
            #pragma omp section
            { /* section A */ }
            #pragma omp section
            { /* section B */ }
        }
    }
}

/* Function using dispatch directive with parallel clause */
#pragma omp declare variant(func_dispatch_variant) match(construct={dispatch}, device={arch(any)})
void func_dispatch_base(void) {
    /* base version */
}

void func_dispatch_variant(void) {
    #pragma omp dispatch parallel
    for (int i = 0; i < 10; i++) {
        /* dispatched loop */
    }
}

/* Main function to ensure all functions are referenced */
int main(void) {
    #pragma omp target
    {
        func_for();
        func_parallel();
        func_sections();
        func_taskgroup();
        func_combined();
        func_dispatch_base();
    }
    
    /* Additional taskgroup in main */
    #pragma omp parallel
    {
        #pragma omp taskgroup
        {
            #pragma omp task
            func_for();
            #pragma omp task
            func_parallel();
        }
    }
    
    return 0;
}
