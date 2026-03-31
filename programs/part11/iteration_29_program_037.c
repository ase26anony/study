/* test_omp_clauses.c
 * Designed to trigger pretty-printing of OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test_omp_clauses.c -o test
 */

/* Function targeted with 'for' clause in declare target */
#pragma omp declare target to(func_for) for
void func_for(int n, double *a, double *b) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        a[i] = b[i] * 2.0;
    }
}

/* Function targeted with 'parallel' clause in declare target */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n, double *a) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        a[tid] = tid * 1.5;
    }
}

/* Function targeted with 'sections' clause in declare target */
#pragma omp declare target to(func_sections) sections
void func_sections(double *x, double *y, double *z) {
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            *x = 1.0;
        }
        #pragma omp section
        {
            *y = 2.0;
        }
        #pragma omp section
        {
            *z = 3.0;
        }
    }
}

/* Function using taskgroup construct */
#pragma omp declare target to(func_taskgroup)
void func_taskgroup(int n, double *data) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    for (int i = 0; i < n/2; i++) {
                        data[i] = i * 0.5;
                    }
                }
                #pragma omp task
                {
                    for (int i = n/2; i < n; i++) {
                        data[i] = i * 0.7;
                    }
                }
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare target to(func_dispatch)
void func_dispatch(int device, int n, double *a) {
    #pragma omp dispatch device(device) parallel
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            a[i] = a[i] + 1.0;
        }
    }
}

/* Function with combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n, double *a, double *b, double *c) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            a[i] = b[i] + c[i];
        }
        
        #pragma omp sections
        {
            #pragma omp section
            { /* dummy section */ }
            #pragma omp section
            { /* dummy section */ }
        }
    }
}

/* Main function that references all functions to prevent dead code elimination */
int main() {
    const int N = 100;
    double a[N], b[N], c[N];
    double x, y, z;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        b[i] = i * 1.0;
        c[i] = i * 0.5;
    }
    
    /* Call functions to ensure they're compiled and used */
    #pragma omp target data map(tofrom: a[0:N])
    {
        func_for(N, a, b);
        func_parallel(N, a);
    }
    
    func_sections(&x, &y, &z);
    func_taskgroup(N, a);
    func_dispatch(0, N, a);
    func_combined(N, a, b, c);
    
    /* Use results to prevent optimization */
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += a[i];
    }
    
    return (int)(sum + x + y + z) % 256;
}
