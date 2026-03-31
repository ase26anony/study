/* test-omp-pretty-print.c
 * 
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clauses (for, parallel, sections, taskgroup) in
 * GCC's tree-pretty-print.cc when compiled with -fdump-tree-* flags.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower test-omp-pretty-print.c -o test
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
            for (int i = 0; i < n; i++) {
                #pragma omp task
                {
                    data[i] = i * 0.5;
                }
            }
            #pragma omp taskgroup
            {
                /* Wait for all tasks created in this group */
                #pragma omp task
                {
                    data[0] = -1.0;  /* Some final task */
                }
            }
        }
    }
}

/* Function using dispatch construct with parallel clause */
#pragma omp declare target to(func_dispatch)
void func_dispatch(int n, double *a, double *b) {
    #pragma omp dispatch parallel for
    for (int i = 0; i < n; i++) {
        a[i] = b[i] + i;
    }
}

/* Combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n, double *arr) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * arr[i];
    }
}

/* Main function that references all targeted functions */
int main() {
    const int N = 100;
    double a[N], b[N], x, y, z;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0;
        b[i] = i * 2.0;
    }
    
    /* Call functions to ensure they're compiled and used */
    #pragma omp target data map(tofrom: a[0:N], b[0:N])
    {
        func_for(N, a, b);
        func_parallel(N, a);
        func_sections(&x, &y, &z);
        func_taskgroup(N, a);
        func_dispatch(N, a, b);
        func_combined(N, a);
    }
    
    /* Use results to prevent dead code elimination */
    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        sum += a[i] + b[i];
    }
    sum += x + y + z;
    
    return (sum > 0) ? 0 : 1;
}
