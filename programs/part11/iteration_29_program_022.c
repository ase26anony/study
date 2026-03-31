/* test-omp-pretty-print.c
 * This program is designed to trigger the pretty-printer logic for
 * specific OpenMP clause names (for, parallel, sections, taskgroup)
 * in GCC's tree-pretty-print.cc when compiled with -fdump-tree-* flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function targeted with 'for' clause in declare target */
#pragma omp declare target to(func_for) for
void func_for(int n, double *a, double *b, double *c) {
    #pragma omp for
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function targeted with 'parallel' clause in declare target */
#pragma omp declare target to(func_parallel) parallel
void func_parallel(int n, double *arr) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        arr[i] *= 2.0;
    }
}

/* Function targeted with 'sections' clause in declare target */
#pragma omp declare target to(func_sections) sections
void func_sections(double *x, double *y, double *z) {
    #pragma omp parallel sections
    {
        #pragma omp section
        { *x = 1.0; }
        #pragma omp section
        { *y = 2.0; }
        #pragma omp section
        { *z = 3.0; }
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
                        data[i] = i * 1.5;
                    }
                }
                #pragma omp task
                {
                    for (int i = n/2; i < n; i++) {
                        data[i] = i * 2.5;
                    }
                }
            }
        }
    }
}

/* Function using dispatch directive with parallel clause */
#pragma omp declare variant(func_parallel) match(construct={dispatch}, device={arch(x86_64)})
void func_dispatch_variant(int n, double *arr) {
    for (int i = 0; i < n; i++) arr[i] = 0.0;
}

/* Combined clauses in declare target */
#pragma omp declare target to(func_combined) for parallel
void func_combined(int n, double *a, double *b) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        a[i] = b[i] * 3.14;
    }
}

/* Main function that references all above functions to ensure they are compiled */
int main(void) {
    const int N = 100;
    double *a = (double *)malloc(N * sizeof(double));
    double *b = (double *)malloc(N * sizeof(double));
    double *c = (double *)malloc(N * sizeof(double));
    double x = 0.0, y = 0.0, z = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (double)i;
        b[i] = (double)(N - i);
        c[i] = 0.0;
    }
    
    /* Call functions to ensure they are referenced and compiled */
    #pragma omp target data map(tofrom: a[0:N], b[0:N], c[0:N])
    {
        func_for(N, a, b, c);
        func_parallel(N, a);
        func_combined(N, b, c);
    }
    
    func_sections(&x, &y, &z);
    func_taskgroup(N, a);
    func_dispatch_variant(N, b);
    
    /* Simple verification */
    printf("Results: x=%.2f, y=%.2f, z=%.2f\n", x, y, z);
    printf("First element of c: %.2f\n", c[0]);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
