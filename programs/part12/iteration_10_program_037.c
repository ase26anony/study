/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: case 0-7 partition string mappings
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 16

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
    
    /* Test data arrays */
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
     * CASE 0: gang redundant
     * Scalar reduction with no data partitioning
     * ============================================ */
    printf("Testing Case 0 (gang redundant)...\n");
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
    
    /* ============================================
     * CASE 1: gang partitioned
     * Array distributed across gangs only
     * ============================================ */
    printf("Testing Case 1 (gang partitioned)...\n");
    #pragma acc parallel copyout(temp[0:N]) copyin(arr1d[0:N])
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
    
    /* ============================================
     * CASE 2: worker partitioned
     * Worker-level distribution without gangs
     * ============================================ */
    printf("Testing Case 2 (worker partitioned)...\n");
    #pragma acc parallel copyout(temp[0:N]) copyin(arr1d[0:N]) num_workers(4)
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
    
    /* ============================================
     * CASE 3: gang+worker partitioned
     * Combined gang and worker partitioning
     * ============================================ */
    printf("Testing Case 3 (gang+worker partitioned)...\n");
    #pragma acc parallel copyout(temp[0:N]) copyin(arr1d[0:N])
    {
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            temp[i] = arr1d[i] * 3.0f;
        }
    }
    if (!verify_array(temp, N, 3.0f)) {
        printf("Case 3 failed\n");
        passed = 0;
    }
    
    /* ============================================
     * CASE 4: vector partitioned
     * Vector-level SIMD partitioning
     * ============================================ */
    printf("Testing Case 4 (vector partitioned)...\n");
    #pragma acc parallel copyout(temp[0:N]) copyin(arr1d[0:N]) vector_length(32)
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
    
    /* ============================================
     * CASE 5: gang+vector partitioned
     * Gang and vector without workers
     * ============================================ */
    printf("Testing Case 5 (gang+vector partitioned)...\n");
    #pragma acc parallel copyout(temp[0:N]) copyin(arr1d[0:N]) vector_length(64)
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
    
    /* ============================================
     * CASE 6: worker+vector partitioned
     * Worker and vector combination
     * ============================================ */
    printf("Testing Case 6 (worker+vector partitioned)...\n");
    #pragma acc parallel copyout(temp[0:N]) copyin(arr1d[0:N]) num_workers(8) vector_length(16)
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
    
    /* ============================================
     * CASE 7: fully partitioned
     * All three levels: gang, worker, vector
     * ============================================ */
    printf("Testing Case 7 (fully partitioned)...\n");
    #pragma acc parallel copyout(temp[0:N]) copyin(arr1d[0:N]) vector_length(32)
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++) {
            temp[i] = arr1d[i] * 7.0f;
        }
    }
    if (!verify_array(temp, N, 7.0f)) {
        printf("Case 7 failed\n");
        passed = 0;
    }
    
    /* ============================================
     * Additional tests for complex scenarios
     * ============================================ */
    
    /* Test with collapse clause for multi-dimensional partitioning */
    printf("Testing multi-dimensional collapse...\n");
    float sum2d = 0.0f;
    #pragma acc parallel copyin(arr2d[0:N*M]) copy(sum2d) reduction(+:sum2d)
    {
        #pragma acc loop gang worker vector collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                sum2d += arr2d[i * M + j];
            }
        }
    }
    if (sum2d != N * M * 2.0f) {
        printf("Collapse test failed: sum = %f, expected %f\n", 
               sum2d, N * M * 2.0f);
        passed = 0;
    }
    
    /* Test with present clause for already-resident data */
    printf("Testing with present clause...\n");
    #pragma acc enter data copyin(arr1d[0:N])
    #pragma acc parallel present(arr1d[0:N]) copyout(temp[0:N])
    {
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            temp[i] = arr1d[i] * 8.0f;
        }
    }
    #pragma acc exit data delete(arr1d[0:N])
    if (!verify_array(temp, N, 8.0f)) {
        printf("Present clause test failed\n");
        passed = 0;
    }
    
    /* Test with private variables */
    printf("Testing with private variables...\n");
    float private_sum = 0.0f;
    #pragma acc parallel copyin(arr1d[0:N]) copy(private_sum) private(temp)
    {
        #pragma acc loop gang reduction(+:private_sum)
        for (i = 0; i < N; i++) {
            private_sum += arr1d[i];
        }
    }
    if (private_sum != N * 1.0f) {
        printf("Private variable test failed\n");
        passed = 0;
    }
    
    /* Test with runtime parameters */
    printf("Testing with runtime parameters...\n");
    int num_gangs = 4;
    int num_workers = 2;
    int vector_len = 16;
    
    #pragma acc parallel copyout(temp[0:N]) copyin(arr1d[0:N]) \
        num_gangs(num_gangs) num_workers(num_workers) vector_length(vector_len)
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++) {
            temp[i] = arr1d[i] * 9.0f;
        }
    }
    if (!verify_array(temp, N, 9.0f)) {
        printf("Runtime parameters test failed\n");
        passed = 0;
    }
    
    /* Test triangular loop (non-rectangular iteration space) */
    printf("Testing triangular loop...\n");
    float tri_sum = 0.0f;
    #pragma acc parallel copy(tri_sum) reduction(+:tri_sum)
    {
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            #pragma acc loop vector
            for (j = 0; j < i; j++) {
                tri_sum += 1.0f;
            }
        }
    }
    float expected_tri = (N * (N - 1)) / 2.0f;
    if (tri_sum != expected_tri) {
        printf("Triangular loop test failed: got %f, expected %f\n", 
               tri_sum, expected_tri);
        passed = 0;
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
        printf("Note: The default case (case 8+) requires compiler-internal\n");
        printf("testing hooks or invalid partition codes to trigger.\n");
        return 0;
    } else {
        printf("\nSome tests failed!\n");
        return 1;
    }
}
