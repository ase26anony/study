/* Test program to cover all partition code cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 128
#define P 32

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double val) {
    for (int i = 0; i < size; i++) {
        arr[i] = val + i * 0.1;
    }
}

/* Helper function to verify results */
int verify_array(double *arr, int size, double expected_base) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        double expected = expected_base + i * 0.1;
        if (abs(arr[i] - expected) > 1e-6) {
            errors++;
        }
    }
    return errors;
}

int main() {
    double *a, *b, *c, *d, *e;
    double sum = 0.0;
    int errors = 0;
    
    /* Allocate and initialize test arrays */
    a = (double*)malloc(N * sizeof(double));
    b = (double*)malloc(N * sizeof(double));
    c = (double*)malloc(N * M * sizeof(double));
    d = (double*)malloc(N * M * P * sizeof(double));
    e = (double*)malloc(N * sizeof(double));
    
    init_array(a, N, 1.0);
    init_array(b, N, 2.0);
    init_array(c, N * M, 3.0);
    init_array(d, N * M * P, 4.0);
    init_array(e, N, 5.0);
    
    printf("Testing OpenACC partition cases...\n");
    
    /* ============================================
     * Case 0: gang redundant
     * Scalar reduction with no data partitioning across gangs
     * ============================================ */
    printf("Testing Case 0 (gang redundant)...\n");
    sum = 0.0;
    #pragma acc parallel copyin(a[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += a[i];
        }
    }
    printf("  Sum = %f\n", sum);
    
    /* ============================================
     * Case 1: gang partitioned
     * Array data distributed across gangs but not within gangs
     * ============================================ */
    printf("Testing Case 1 (gang partitioned)...\n");
    #pragma acc parallel loop gang copy(a[0:N], b[0:N])
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 2.0;
    }
    errors = verify_array(b, N, 2.0);
    printf("  Errors in b: %d\n", errors);
    
    /* ============================================
     * Case 2: worker partitioned
     * Worker-level distribution
     * ============================================ */
    printf("Testing Case 2 (worker partitioned)...\n");
    #pragma acc parallel loop worker num_workers(4) copy(a[0:N], e[0:N])
    for (int i = 0; i < N; i++) {
        e[i] = a[i] + 1.0;
    }
    errors = verify_array(e, N, 2.0);
    printf("  Errors in e: %d\n", errors);
    
    /* ============================================
     * Case 3: gang+worker partitioned
     * Combine gang and worker partitioning with nested loops
     * ============================================ */
    printf("Testing Case 3 (gang+worker partitioned)...\n");
    #pragma acc parallel loop gang worker collapse(2) copy(c[0:N*M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = c[idx] * 1.5;
        }
    }
    errors = verify_array(c, N * M, 4.5);
    printf("  Errors in c: %d\n", errors);
    
    /* ============================================
     * Case 4: vector partitioned
     * Vector-level partitioning with SIMD-style operations
     * ============================================ */
    printf("Testing Case 4 (vector partitioned)...\n");
    #pragma acc parallel loop vector vector_length(32) copy(a[0:N], b[0:N])
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 3.0;
    }
    errors = verify_array(b, N, 3.0);
    printf("  Errors in b: %d\n", errors);
    
    /* ============================================
     * Case 5: gang+vector partitioned
     * Combine gang and vector partitioning without worker involvement
     * ============================================ */
    printf("Testing Case 5 (gang+vector partitioned)...\n");
    #pragma acc parallel loop gang vector copy(a[0:N], b[0:N])
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 4.0;
    }
    errors = verify_array(b, N, 4.0);
    printf("  Errors in b: %d\n", errors);
    
    /* ============================================
     * Case 6: worker+vector partitioned
     * Combine worker and vector partitioning
     * ============================================ */
    printf("Testing Case 6 (worker+vector partitioned)...\n");
    #pragma acc parallel loop worker vector num_workers(4) vector_length(16) copy(a[0:N], e[0:N])
    for (int i = 0; i < N; i++) {
        e[i] = a[i] * 5.0;
    }
    errors = verify_array(e, N, 5.0);
    printf("  Errors in e: %d\n", errors);
    
    /* ============================================
     * Case 7: fully partitioned
     * Use all three levels: gang, worker, vector
     * ============================================ */
    printf("Testing Case 7 (fully partitioned)...\n");
    #pragma acc parallel loop gang worker vector collapse(3) copy(d[0:N*M*P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                int idx = (i * M + j) * P + k;
                d[idx] = d[idx] * 2.0;
            }
        }
    }
    errors = verify_array(d, N * M * P, 8.0);
    printf("  Errors in d: %d\n", errors);
    
    /* ============================================
     * Additional tests with different data mappings
     * to ensure comprehensive coverage
     * ============================================ */
    
    /* Test with copyin/copyout */
    printf("Testing with copyin/copyout...\n");
    double *f = (double*)malloc(N * sizeof(double));
    init_array(f, N, 10.0);
    
    #pragma acc parallel loop gang copyin(f[0:N]) copyout(b[0:N])
    for (int i = 0; i < N; i++) {
        b[i] = f[i] * 2.0;
    }
    errors = verify_array(b, N, 20.0);
    printf("  Errors with copyin/copyout: %d\n", errors);
    
    /* Test with present clause simulation */
    printf("Testing data reuse...\n");
    #pragma acc data copy(a[0:N], b[0:N])
    {
        #pragma acc parallel loop gang present(a, b)
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 6.0;
        }
        
        #pragma acc parallel loop worker vector present(a, b)
        for (int i = 0; i < N; i++) {
            a[i] = b[i] / 2.0;
        }
    }
    errors = verify_array(a, N, 3.0);
    printf("  Errors with data reuse: %d\n", errors);
    
    /* Test with private variables */
    printf("Testing with private variables...\n");
    double private_sum = 0.0;
    #pragma acc parallel loop gang reduction(+:private_sum) private(b) copy(a[0:N])
    for (int i = 0; i < N; i++) {
        double local_var = a[i];
        private_sum += local_var;
    }
    printf("  Private sum = %f\n", private_sum);
    
    /* Test with runtime parameters */
    printf("Testing with runtime parameters...\n");
    int num_gangs = 8;
    int num_workers = 4;
    int vec_len = 32;
    
    #pragma acc parallel loop gang worker vector num_gangs(num_gangs) num_workers(num_workers) vector_length(vec_len) copy(a[0:N], b[0:N])
    for (int i = 0; i < N; i++) {
        b[i] = a[i] * 7.0;
    }
    errors = verify_array(b, N, 7.0);
    printf("  Errors with runtime params: %d\n", errors);
    
    /* Test triangular loop */
    printf("Testing triangular loop...\n");
    #pragma acc parallel loop gang
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < i; j++) {
            int idx = i * N + j;
            if (idx < N * N) {
                /* Access pattern creates interesting partitioning */
            }
        }
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
    
    printf("\nAll partition tests completed.\n");
    printf("To trigger the default case (<illegal>), the compiler would need\n");
    printf("to be tested with invalid partition codes through internal APIs.\n");
    
    return 0;
}
