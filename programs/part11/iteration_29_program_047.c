/* test_omp_clauses.c
 * This program is designed to trigger pretty-printing of specific OpenMP clauses:
 * for, parallel, sections, and taskgroup.
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
        int id = omp_get_thread_num();
        /* parallel work */
    }
}

/* Function targeted with 'sections' clause in declare target */
#pragma omp declare target to(func_sections) sections
void func_sections(void) {
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            /* section 1 work */
        }
        #pragma omp section
        {
            /* section 2 work */
        }
    }
}

/* Function using taskgroup construct */
#pragma omp declare target to(func_taskgroup)
void func_taskgroup(void) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    /* task work */
                }
                #pragma omp task
                {
                    /* more task work */
                }
            }
        }
    }
}

/* Combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(void) {
    #pragma omp parallel for
    for (int i = 0; i < 10; i++) {
        /* combined parallel for work */
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare variant(func_parallel) match(construct={dispatch})
void func_dispatch(void) {
    #pragma omp dispatch
    func_parallel();
}

/* Main function to ensure all functions are referenced */
int main(void) {
    #pragma omp target teams
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
