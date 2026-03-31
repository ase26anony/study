/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 32

/* Helper function to verify results */
int verify_array(float *arr, int size, float expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            printf("Verification failed at index %d: got %f, expected %f\n", 
                   i, arr[i], expected);
            return 0;
        }
    }
    return 1;
}

int main() {
    int i, j, k;
    int passed = 1;
    
    /* Test arrays */
    float *arr1d = (float*)malloc(N * sizeof(float));
    float *arr2d = (float*)malloc(N * M * sizeof(float));
    float *arr3d = (float*)malloc(N * M * P * sizeof(float));
    float *temp = (float*)malloc(N * sizeof(float));
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = 1.0f;
        temp[i] = 0.0f;
    }
    
    for (i = 0; i < N * M; i++) {
        arr2d[i] = 2.0f;
    }
    
    for (i = 0; i < N * M * P; i++) {
        arr3d[i] = 3.0f;
    }
    
    printf("Testing OpenACC partition cases...\n");
    
    /* ============================================
     * Case 0: gang redundant
     * Scalar reduction with no data partitioning across gangs
     * ============================================ */
    printf("Testing case 0 (gang redundant)...\n");
    {
        float sum = 0.0f;
        #pragma acc parallel copyin(arr1d[0:N]) copy(sum) reduction(+:sum)
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                sum += arr1d[i];
            }
        }
        if (sum != N * 1.0f) {
            printf("Case 0 failed: sum = %f, expected %f\n", sum, N * 1.0f);
            passed = 0;
        }
    }
    
    /* ============================================
     * Case 1: gang partitioned
     * Array data distributed across gangs but not within gangs
     * ============================================ */
    printf("Testing case 1 (gang partitioned)...\n");
    {
        #pragma acc parallel copy(arr1d[0:N]) copyout(temp[0:N])
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                temp[i] = arr1d[i] * 2.0f;
            }
        }
        if (!verify_array(temp, N, 2.0f)) {
            printf("Case 1 failed\n");
            passed = 0;
        }
    }
    
    /* ============================================
     * Case 2: worker partitioned
     * Worker-level distribution
     * ============================================ */
    printf("Testing case 2 (worker partitioned)...\n");
    {
        #pragma acc parallel copy(arr1d[0:N]) copyout(temp[0:N]) num_workers(4)
        {
            #pragma acc loop worker
            for (i = 0; i < N; i++) {
                temp[i] = arr1d[i] + 1.0f;
            }
        }
        if (!verify_array(temp, N, 2.0f)) {
            printf("Case 2 failed\n");
            passed = 0;
        }
    }
    
    /* ============================================
     * Case 3: gang+worker partitioned
     * Combine gang and worker partitioning
     * ============================================ */
    printf("Testing case 3 (gang+worker partitioned)...\n");
    {
        #pragma acc parallel copy(arr2d[0:N*M]) copyout(temp[0:N*M])
        {
            #pragma acc loop gang worker collapse(2)
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    temp[i * M + j] = arr2d[i * M + j] * 3.0f;
                }
            }
        }
        if (!verify_array(temp, N * M, 6.0f)) {
            printf("Case 3 failed\n");
            passed = 0;
        }
    }
    
    /* ============================================
     * Case 4: vector partitioned
     * Vector-level partitioning
     * ============================================ */
    printf("Testing case 4 (vector partitioned)...\n");
    {
        #pragma acc parallel copy(arr1d[0:N]) copyout(temp[0:N]) vector_length(32)
        {
            #pragma acc loop vector
            for (i = 0; i < N; i++) {
                temp[i] = arr1d[i] * 4.0f;
            }
        }
        if (!verify_array(temp, N, 4.0f)) {
            printf("Case 4 failed\n");
            passed = 0;
        }
    }
    
    /* ============================================
     * Case 5: gang+vector partitioned
     * Combine gang and vector partitioning
     * ============================================ */
    printf("Testing case 5 (gang+vector partitioned)...\n");
    {
        #pragma acc parallel copy(arr1d[0:N]) copyout(temp[0:N])
        {
            #pragma acc loop gang vector
            for (i = 0; i < N; i++) {
                temp[i] = arr1d[i] * 5.0f;
            }
        }
        if (!verify_array(temp, N, 5.0f)) {
            printf("Case 5 failed\n");
            passed = 0;
        }
    }
    
    /* ============================================
     * Case 6: worker+vector partitioned
     * Combine worker and vector partitioning
     * ============================================ */
    printf("Testing case 6 (worker+vector partitioned)...\n");
    {
        #pragma acc parallel copy(arr1d[0:N]) copyout(temp[0:N]) num_workers(4) vector_length(16)
        {
            #pragma acc loop worker vector
            for (i = 0; i < N; i++) {
                temp[i] = arr1d[i] * 6.0f;
            }
        }
        if (!verify_array(temp, N, 6.0f)) {
            printf("Case 6 failed\n");
            passed = 0;
        }
    }
    
    /* ============================================
     * Case 7: fully partitioned
     * All three levels: gang, worker, vector
     * ============================================ */
    printf("Testing case 7 (fully partitioned)...\n");
    {
        float *output = (float*)malloc(N * M * P * sizeof(float));
        
        #pragma acc parallel copyin(arr3d[0:N*M*P]) copyout(output[0:N*M*P])
        {
            #pragma acc loop gang worker vector collapse(3)
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    for (k = 0; k < P; k++) {
                        int idx = (i * M * P) + (j * P) + k;
                        output[idx] = arr3d[idx] * 2.0f;
                    }
                }
            }
        }
        
        if (!verify_array(output, N * M * P, 6.0f)) {
            printf("Case 7 failed\n");
            passed = 0;
        }
        
        free(output);
    }
    
    /* ============================================
     * Additional tests with different data mappings
     * to influence partitioning decisions
     * ============================================ */
    
    /* Test with present clause */
    printf("Testing with present clause...\n");
    {
        #pragma acc enter data copyin(arr1d[0:N])
        
        #pragma acc parallel present(arr1d[0:N]) copyout(temp[0:N])
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                temp[i] = arr1d[i] * 7.0f;
            }
        }
        
        #pragma acc exit data delete(arr1d[0:N])
        
        if (!verify_array(temp, N, 7.0f)) {
            printf("Present clause test failed\n");
            passed = 0;
        }
    }
    
    /* Test with private variables */
    printf("Testing with private variables...\n");
    {
        float private_var = 10.0f;
        #pragma acc parallel copyout(temp[0:N]) private(private_var)
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                private_var = i * 1.0f;
                temp[i] = private_var;
            }
        }
        /* Note: private variables are undefined after parallel region */
    }
    
    /* Test with reduction on array sections */
    printf("Testing array reduction...\n");
    {
        float sum_array[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        #pragma acc parallel copyin(arr1d[0:N]) copy(sum_array[0:4])
        {
            #pragma acc loop gang reduction(+:sum_array[0:4])
            for (i = 0; i < N; i++) {
                sum_array[i % 4] += arr1d[i];
            }
        }
        float expected_sum = (N / 4.0f) * 1.0f;
        for (i = 0; i < 4; i++) {
            if (sum_array[i] != expected_sum && sum_array[i] != expected_sum + 1.0f) {
                printf("Array reduction test failed at index %d: %f\n", i, sum_array[i]);
                passed = 0;
            }
        }
    }
    
    /* Test with runtime parameters */
    printf("Testing with runtime parameters...\n");
    {
        int num_gangs = 8;
        int num_workers = 4;
        int vector_len = 32;
        
        #pragma acc parallel copy(arr1d[0:N]) copyout(temp[0:N]) \
                    num_gangs(num_gangs) num_workers(num_workers) vector_length(vector_len)
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < N; i++) {
                temp[i] = arr1d[i] * 8.0f;
            }
        }
        
        if (!verify_array(temp, N, 8.0f)) {
            printf("Runtime parameters test failed\n");
            passed = 0;
        }
    }
    
    /* Test triangular loop */
    printf("Testing triangular loop...\n");
    {
        #pragma acc parallel copyout(temp[0:N])
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                #pragma acc loop worker
                for (j = 0; j <= i; j++) {
                    if (j < N) {
                        temp[j] = 9.0f;
                    }
                }
            }
        }
    }
    
    /* Cleanup */
    free(arr1d);
    free(arr2d);
    free(arr3d);
    free(temp);
    
    /* Summary */
    if (passed) {
        printf("\nAll tests passed successfully!\n");
        printf("All partition cases (0-7) should have been exercised.\n");
        printf("Note: The default case (<illegal>) requires compiler-internal\n");
        printf("testing hooks or invalid partition codes to trigger.\n");
        return 0;
    } else {
        printf("\nSome tests failed!\n");
        return 1;
    }
}
