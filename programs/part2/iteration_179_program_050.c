/* test_omp_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -c test_omp_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define M 100

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int target_data[N];
#pragma omp end declare target

/* For OMP_CLAUSE__REDUCTEMP_ with custom reduction */
#pragma omp declare reduction(vec_add : int [M] : \
    for (int i = 0; i < M; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

/* Volatile variables to prevent optimization */
volatile int sink;

int main(int argc, char **argv) {
    int i, j;
    int sum = 0;
    int max_val = -1000000;
    int min_val = 1000000;
    int scan_sum = 0;
    int array_sum[M] = {0};
    
    /* Runtime-dependent iteration count */
    int iterations = (argc > 1) ? atoi(argv[1]) : N;
    if (iterations < 1) iterations = N;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:N])
    
    /* Complex reduction with if clause - may generate _reductemp_ and _condtemp_ */
    /* Use volatile condition to force temporary creation */
    volatile int threshold = 500;
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) reduction(min:min_val) \
        if(iterations > threshold) map(tofrom: sum, max_val, min_val)
    for (i = 0; i < iterations; i++) {
        int val = target_data[i];
        sum += val;
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    sink = sum + max_val + min_val; /* Prevent dead code elimination */
    
    /* Scan directive - triggers OMP_CLAUSE__SCANTEMP_ */
    int scan_array[N];
    for (i = 0; i < N; i++) scan_array[i] = i % 10;
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        private(i) schedule(static, 10)
    for (i = 0; i < N; i++) {
        scan_sum += scan_array[i];
        #pragma omp scan inclusive(scan_sum)
        scan_array[i] = scan_sum;
    }
    
    sink = scan_sum + scan_array[N-1];
    
    /* Nested reductions with array reduction - may generate multiple _reductemp_ */
    int data[N][M];
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = (i + j) % 10;
        }
    }
    
    #pragma omp parallel for reduction(vec_add: array_sum) \
        reduction(+:sum) collapse(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            array_sum[j] += data[i][j];
            sum += data[i][j];
        }
    }
    
    /* Final conditional parallel region */
    volatile int use_parallel = 1;
    #pragma omp parallel for if(use_parallel) reduction(+:sum) \
        schedule(dynamic, 5)
    for (i = 0; i < iterations/2; i++) {
        sum += i;
    }
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:N])
    
    /* Print results to ensure side effects */
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d, array_sum[0]=%d\n",
           sum, max_val, min_val, scan_sum, array_sum[0]);
    
    return 0;
}
