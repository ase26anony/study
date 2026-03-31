/* test-omp-pretty-print.c
 * This program is designed to trigger coverage of specific OMP clause
 * pretty-printing code in GCC's tree-pretty-print.cc (lines 1434-1445).
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-pretty-print.c -o test
 */

/* Function targeted with 'for' clause in declare target */
#pragma omp declare target to(func_for) for
void func_for(void) {
    int i;
    #pragma omp for
    for (i = 0; i < 10; i++) {
        /* empty loop body */
    }
}

/* Function targeted with 'parallel' clause in declare target */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(void) {
    #pragma omp parallel
    {
        /* empty parallel region */
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
        { /* task inside taskgroup */ }
    }
}

/* Function using combined clauses in declare target */
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

/* Function using dispatch directive with parallel clause */
void func_dispatch(void) {
    int device_id = 0;
    #pragma omp dispatch device(device_id) parallel
    for (int i = 0; i < 10; i++) {
        /* loop body */
    }
}

/* Main function calls all to ensure they're compiled */
int main(void) {
    #pragma omp target
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
