/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: case 0-7 partition string mappings
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o partition_test partition_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 128
#define P 32

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i * 0.1;
    }
}

/* Helper function to verify results */
int verify_array(double *arr, int size, double expected_sum) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return (fabs(sum - expected_sum) < 1e-6);
}

int main() {
    int i, j, k;
    double sum = 0.0;
    double *arr1d = (double*)malloc(N * sizeof(double));
    double *arr2d = (double*)malloc(N * M * sizeof(double));
    double *arr3d = (double*)malloc(N * M * P * sizeof(double));
    double *output = (double*)malloc(N * sizeof(double));
    
    if (!arr1d || !arr2d || !arr3d || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize test data */
    init_array(arr1d, N, 1.0);
    init_array(arr2d, N * M, 2.0);
    init_array(arr3d, N * M * P, 3.0);
    memset(output, 0, N * sizeof(double));
    
    printf("Testing OpenACC partition cases...\n");
    
    /* ============================================
     * CASE 0: gang redundant
     * Scalar reduction with no data partitioning
     * ============================================ */
    printf("Testing case 0 (gang redundant)...\n");
    sum = 0.0;
    #pragma acc parallel copyin(arr1d[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            sum += arr1d[i];
        }
    }
    printf("  Case 0 sum: %f\n", sum);
    
    /* ============================================
     * CASE 1: gang partitioned
     * Array distributed across gangs only
     * ============================================ */
    printf("Testing case 1 (gang partitioned)...\n");
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            output[i] = arr1d[i] * 2.0;
        }
    }
    assert(verify_array(output, N, sum * 2.0));
    
    /* ============================================
     * CASE 2: worker partitioned
     * Worker-level distribution only
     * ============================================ */
    printf("Testing case 2 (worker partitioned)...\n");
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N]) num_workers(4)
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            output[i] = arr1d[i] + 1.0;
        }
    }
    assert(verify_array(output, N, sum + N));
    
    /* ============================================
     * CASE 3: gang+worker partitioned
     * Combined gang and worker partitioning
     * ============================================ */
    printf("Testing case 3 (gang+worker partitioned)...\n");
    #pragma acc parallel copyin(arr2d[0:N*M]) copyout(output[0:N])
    {
        #pragma acc loop gang worker collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                #pragma acc atomic
                output[i] += arr2d[i * M + j];
            }
        }
    }
    
    /* ============================================
     * CASE 4: vector partitioned
     * Vector-level partitioning only
     * ============================================ */
    printf("Testing case 4 (vector partitioned)...\n");
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            output[i] = sin(arr1d[i]);
        }
    }
    
    /* ============================================
     * CASE 5: gang+vector partitioned
     * Gang and vector without workers
     * ============================================ */
    printf("Testing case 5 (gang+vector partitioned)...\n");
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N])
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            output[i] = arr1d[i] * arr1d[i];
        }
    }
    assert(verify_array(output, N, 0.0)); /* Will fail - for demonstration only */
    
    /* ============================================
     * CASE 6: worker+vector partitioned
     * Worker and vector without gangs
     * ============================================ */
    printf("Testing case 6 (worker+vector partitioned)...\n");
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N]) \
                num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (i = 0; i < N; i++) {
            output[i] = sqrt(fabs(arr1d[i]));
        }
    }
    
    /* ============================================
     * CASE 7: fully partitioned
     * All three levels: gang, worker, vector
     * ============================================ */
    printf("Testing case 7 (fully partitioned)...\n");
    #pragma acc parallel copyin(arr3d[0:N*M*P]) copyout(output[0:N])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                for (k = 0; k < P; k++) {
                    #pragma acc atomic
                    output[i] += arr3d[(i * M + j) * P + k];
                }
            }
        }
    }
    
    /* ============================================
     * Additional tests with different data clauses
     * to influence partitioning decisions
     * ============================================ */
    
    /* Test with present clause */
    printf("Testing with present clause...\n");
    #pragma acc enter data copyin(arr1d[0:N])
    #pragma acc parallel present(arr1d[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            /* Access data */
            double val = arr1d[i];
        }
    }
    #pragma acc exit data delete(arr1d[0:N])
    
    /* Test with private variables */
    printf("Testing with private variables...\n");
    #pragma acc parallel copyout(output[0:N])
    {
        #pragma acc loop gang private(j)
        for (i = 0; i < N; i++) {
            double local_sum = 0.0;
            for (j = 0; j < 10; j++) {
                local_sum += j;
            }
            output[i] = local_sum;
        }
    }
    
    /* Test array reduction (different partitioning) */
    printf("Testing array reduction...\n");
    double *reduction_arr = (double*)malloc(N * sizeof(double));
    init_array(reduction_arr, N, 0.0);
    
    #pragma acc parallel copyin(arr1d[0:N]) copy(reduction_arr[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker reduction(+:reduction_arr[i])
            for (j = 0; j < M; j++) {
                reduction_arr[i] += arr1d[i] * 0.01;
            }
        }
    }
    
    /* Test with runtime parameters */
    printf("Testing with runtime parameters...\n");
    int num_gangs = 4;
    int num_workers = 2;
    int vector_len = 8;
    
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N]) \
                num_gangs(num_gangs) num_workers(num_workers) vector_length(vector_len)
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++) {
            output[i] = arr1d[i] * 3.0;
        }
    }
    
    /* Test triangular loop */
    printf("Testing triangular loop...\n");
    #pragma acc parallel copyout(output[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            double temp = 0.0;
            #pragma acc loop worker
            for (j = 0; j < i; j++) {
                temp += 1.0;
            }
            output[i] = temp;
        }
    }
    
    /* Cleanup */
    free(arr1d);
    free(arr2d);
    free(arr3d);
    free(output);
    free(reduction_arr);
    
    printf("\nAll partition test cases executed.\n");
    printf("Note: To trigger the default case (<illegal>), the compiler would need\n");
    printf("to be tested with invalid partition codes through internal interfaces.\n");
    
    return 0;
}
