/* test_omp_clauses.c - Coverage for OMP clause pretty-printing */

/* Function targeted with 'for' clause */
#pragma omp declare target to(func_for) for
void func_for(void) {
    int i;
    #pragma omp parallel for
    for (i = 0; i < 100; i++) {
        /* Some computation */
        volatile int x = i * 2;
    }
}

/* Function targeted with 'parallel' clause */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        volatile int work = tid * 100;
    }
}

/* Function targeted with 'sections' clause */
#pragma omp declare target to(func_sections) sections
void func_sections(void) {
    #pragma omp parallel sections
    {
        #pragma omp section
        { volatile int a = 1; }
        
        #pragma omp section
        { volatile int b = 2; }
    }
}

/* Function using taskgroup clause */
#pragma omp declare target to(func_taskgroup)
void func_taskgroup(void) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                { volatile int t1 = 1; }
                
                #pragma omp task
                { volatile int t2 = 2; }
            }
        }
    }
}

/* Function with combined clauses */
#pragma omp declare target to(func_combined) for parallel
void func_combined(void) {
    #pragma omp parallel for
    for (int i = 0; i < 50; i++) {
        volatile int y = i * 3;
    }
}

/* Function using dispatch directive with parallel clause */
#pragma omp declare variant(func_parallel) match(construct={dispatch}, device={arch(x86_64)})
void func_dispatch(void) {
    #pragma omp dispatch
    func_parallel();
}

/* Main function that references all to ensure compilation */
int main(void) {
    #pragma omp target teams map(tofrom: func_for)
    func_for();
    
    func_parallel();
    func_sections();
    func_taskgroup();
    func_combined();
    func_dispatch();
    
    return 0;
}
