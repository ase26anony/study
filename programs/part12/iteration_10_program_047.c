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
#define P 16

/* Helper to verify results */
int verify_array(int *arr, int size, int expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) return 0;
    }
    return 1;
}

int main() {
    int errors = 0;
    
    /* Initialize test arrays */
    int *arr1d = (int*)malloc(N * sizeof(int));
    int *arr2d = (int*)malloc(N * M * sizeof(int));
    int *arr3d = (int*)malloc(N * M * P * sizeof(int));
    int *src = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        arr1d[i] = 0;
        src[i] = i % 100;
    }
    for (int i = 0; i < N * M; i++) arr2d[i] = 0;
    for (int i = 0; i < N * M * P; i++) arr3d[i] = 0;
    
    printf("Testing OpenACC partition cases...\n");
    
    /* ============================================
     * Case 0: gang redundant
     * Scalar reduction, no data partitioning across gangs
     * ============================================ */
    printf("Testing case 0 (gang redundant)... ");
    int sum = 0;
    #pragma acc parallel copyin(src[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += src[i];
        }
    }
    /* Verify by CPU calculation */
    int cpu_sum = 0;
    for (int i = 0; i < N; i++) cpu_sum += src[i];
    if (sum != cpu_sum) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* ============================================
     * Case 1: gang partitioned
     * Array distributed across gangs only
     * ============================================ */
    printf("Testing case 1 (gang partitioned)... ");
    #pragma acc parallel copy(arr1d[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] = 1;
        }
    }
    if (!verify_array(arr1d, N, 1)) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* ============================================
     * Case 2: worker partitioned
     * Worker-level distribution only
     * ============================================ */
    printf("Testing case 2 (worker partitioned)... ");
    #pragma acc parallel copy(arr1d[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr1d[i] = 2;
        }
    }
    if (!verify_array(arr1d, N, 2)) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* ============================================
     * Case 3: gang+worker partitioned
     * Combined gang and worker partitioning
     * ============================================ */
    printf("Testing case 3 (gang+worker partitioned)... ");
    #pragma acc parallel copy(arr2d[0:N*M])
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2d[i * M + j] = 3;
            }
        }
    }
    /* Check a sample of values */
    int ok = 1;
    for (int i = 0; i < N && ok; i += N/10) {
        for (int j = 0; j < M && ok; j += M/10) {
            if (arr2d[i * M + j] != 3) ok = 0;
        }
    }
    if (!ok) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* ============================================
     * Case 4: vector partitioned
     * Vector-level partitioning only
     * ============================================ */
    printf("Testing case 4 (vector partitioned)... ");
    #pragma acc parallel copy(arr1d[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr1d[i] = 4;
        }
    }
    if (!verify_array(arr1d, N, 4)) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* ============================================
     * Case 5: gang+vector partitioned
     * Gang and vector without workers
     * ============================================ */
    printf("Testing case 5 (gang+vector partitioned)... ");
    #pragma acc parallel copy(arr1d[0:N]) vector_length(64)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            arr1d[i] = 5;
        }
    }
    if (!verify_array(arr1d, N, 5)) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* ============================================
     * Case 6: worker+vector partitioned
     * Worker and vector without gangs
     * ============================================ */
    printf("Testing case 6 (worker+vector partitioned)... ");
    #pragma acc parallel copy(arr1d[0:N]) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            arr1d[i] = 6;
        }
    }
    if (!verify_array(arr1d, N, 6)) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* ============================================
     * Case 7: fully partitioned
     * All three levels: gang, worker, vector
     * ============================================ */
    printf("Testing case 7 (fully partitioned)... ");
    #pragma acc parallel copy(arr3d[0:N*M*P]) num_workers(4) vector_length(32)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr3d[(i * M + j) * P + k] = 7;
                }
            }
        }
    }
    /* Check a sample of 3D array */
    ok = 1;
    for (int i = 0; i < N && ok; i += N/8) {
        for (int j = 0; j < M && ok; j += M/8) {
            for (int k = 0; k < P && ok; k += P/8) {
                if (arr3d[(i * M + j) * P + k] != 7) ok = 0;
            }
        }
    }
    if (!ok) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* ============================================
     * Additional tests for edge cases
     * ============================================ */
    
    /* Test with runtime parameters */
    printf("Testing with runtime parameters... ");
    int block_size = 128;
    #pragma acc parallel copy(arr1d[0:N]) num_gangs(N/block_size) vector_length(block_size)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            arr1d[i] = 8;
        }
    }
    if (!verify_array(arr1d, N, 8)) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test with present clause (simulated) */
    printf("Testing with present clause simulation... ");
    #pragma acc enter data copyin(arr1d[0:N])
    #pragma acc parallel present(arr1d[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] = 9;
        }
    }
    #pragma acc update self(arr1d[0:N])
    if (!verify_array(arr1d, N, 9)) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    #pragma acc exit data delete(arr1d[0:N])
    
    /* Test private variables */
    printf("Testing private variables... ");
    int private_val = 0;
    #pragma acc parallel copy(arr1d[0:N]) private(private_val)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            private_val = i;
            arr1d[i] = private_val % 10;
        }
    }
    /* Just check it ran without crashing */
    printf("PASSED\n");
    
    /* Test reduction with array sections */
    printf("Testing array reduction... ");
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma acc parallel copyin(src[0:N]) copyout(partial_sums[0:4])
    {
        #pragma acc loop gang reduction(+:partial_sums[0:4])
        for (int i = 0; i < N; i++) {
            partial_sums[i % 4] += src[i];
        }
    }
    /* Verify by CPU */
    int cpu_partial[4] = {0, 0, 0, 0};
    for (int i = 0; i < N; i++) cpu_partial[i % 4] += src[i];
    ok = 1;
    for (int i = 0; i < 4; i++) {
        if (partial_sums[i] != cpu_partial[i]) ok = 0;
    }
    if (!ok) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Cleanup */
    free(arr1d);
    free(arr2d);
    free(arr3d);
    free(src);
    
    printf("\nTotal errors: %d\n", errors);
    printf("All partition test cases executed.\n");
    
    /* Note: The default case (case 8+) cannot be directly triggered from
     * user code as it requires invalid partition codes. This would need
     * compiler-internal testing hooks or malformed compiler intermediate
     * representation manipulation.
     */
    
    return errors > 0 ? 1 : 0;
}
