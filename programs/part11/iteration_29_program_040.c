/* test-omp-clauses.c
 * Designed to trigger pretty-printing of OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP in GCC's tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-clauses.c -o test
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

/* Function targeted with combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n, double *a, double *b, double *c) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            a[i] = i * 0.5;
        }
        
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    *b = 10.0;
                }
                #pragma omp task
                {
                    *c = 20.0;
                }
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare variant(func_dispatch_variant) match(construct={dispatch}, device={arch(x86_64)})
void func_dispatch_variant(int n, double *a) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        a[i] = a[i] * 3.0;
    }
}

#pragma omp declare target to(func_dispatch_variant) parallel
void func_dispatch(int n, double *a) {
    #pragma omp dispatch
    func_dispatch_variant(n, a);
}

/* Function with explicit taskgroup construct */
void func_taskgroup(int n, double *results) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (int i = 0; i < n; i++) {
                    #pragma omp task firstprivate(i)
                    {
                        results[i] = i * i * 1.0;
                    }
                }
            }
        }
    }
}

/* Main function to ensure all functions are referenced */
int main() {
    const int N = 100;
    double a[N], b[N], c[N];
    double x, y, z;
    double results[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0;
        b[i] = i * 2.0;
        c[i] = 0.0;
        results[i] = 0.0;
    }
    
    /* Call functions to ensure they're compiled and used */
    #pragma omp target data map(tofrom: a[0:N], b[0:N], c[0:N])
    {
        func_for(N, a, b);
        func_parallel(N, c);
        func_sections(&x, &y, &z);
        func_combined(N, a, b, &x);
        func_dispatch(N, a);
    }
    
    func_taskgroup(N, results);
    
    /* Use results to prevent optimization */
    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i] + results[i];
    }
    
    return (sum > 0.0) ? 0 : 1;
}
