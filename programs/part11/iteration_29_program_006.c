/* test-omp-pretty-print.c
 * 
 * This program is designed to trigger the uncovered pretty-printing
 * logic for OpenMP clauses 'for', 'parallel', 'sections', and 'taskgroup'
 * in GCC's tree-pretty-print.cc when compiled with -fdump-tree-* flags.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-pretty-print.c -o test
 */

/* Function targeted with 'declare target for' clause */
#pragma omp declare target to(func_for) for
void func_for(void) {
    int i;
    #pragma omp for
    for (i = 0; i < 10; i++) {
        /* do work */
    }
}

/* Function targeted with 'declare target parallel' clause */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(void) {
    #pragma omp parallel
    {
        /* parallel region */
    }
}

/* Function targeted with 'declare target sections' clause */
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

/* Combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel sections
void func_combined(void) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < 5; i++) { }
        
        #pragma omp sections
        {
            #pragma omp section
            { }
            #pragma omp section
            { }
        }
    }
}

/* Use dispatch construct with parallel clause */
void func_dispatch(void) {
    int device_id = 0;
    #pragma omp dispatch device(device_id) parallel
    for (int i = 0; i < 8; i++) {
        /* dispatched loop */
    }
}

/* Main function to ensure all functions are referenced */
int main(void) {
    #pragma omp target teams map(tofrom: func_for, func_parallel, func_sections, func_taskgroup, func_combined)
    {
        func_for();
        func_parallel();
        func_sections();
        func_taskgroup();
        func_combined();
    }
    
    func_dispatch();
    
    return 0;
}
