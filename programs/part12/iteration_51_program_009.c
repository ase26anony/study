// test_omp_oacc_partition_types.c
// Compile with: gcc -fopenacc -O2 test_omp_oacc_partition_types.c -o test_partition

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define ARRAY_SIZE 1024

// Helper function to verify computations
void verify_array(float *arr, int size, float expected_value) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected_value) {
            printf("Error at index %d: expected %f, got %f\n", i, expected_value, arr[i]);
            break;
        }
    }
}

int main() {
    float *data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    printf("Testing OpenACC partition types...\n");

    // Case 0: gang redundant
    printf("Testing Case 0: gang redundant\n");
    #pragma acc parallel loop gang(redundant) copyout(data[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 1.0f;
    }
    verify_array(data, 10, 1.0f); // Check first 10 elements

    // Case 1: gang partitioned
    printf("Testing Case 1: gang partitioned\n");
    #pragma acc parallel loop gang(num:32) copyout(data[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 2.0f;
    }
    verify_array(data, 10, 2.0f);

    // Case 2: worker partitioned
    printf("Testing Case 2: worker partitioned\n");
    #pragma acc parallel loop worker(num:4) copyout(data[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 3.0f;
    }
    verify_array(data, 10, 3.0f);

    // Case 3: gang+worker partitioned
    printf("Testing Case 3: gang+worker partitioned\n");
    #pragma acc parallel loop gang(num:16) worker(num:8) copyout(data[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 4.0f;
    }
    verify_array(data, 10, 4.0f);

    // Case 4: vector partitioned
    printf("Testing Case 4: vector partitioned\n");
    #pragma acc parallel loop vector_length(128) copyout(data[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 5.0f;
    }
    verify_array(data, 10, 5.0f);

    // Case 5: gang+vector partitioned
    printf("Testing Case 5: gang+vector partitioned\n");
    #pragma acc parallel loop gang(num:8) vector_length(64) copyout(data[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 6.0f;
    }
    verify_array(data, 10, 6.0f);

    // Case 6: worker+vector partitioned
    printf("Testing Case 6: worker+vector partitioned\n");
    #pragma acc parallel loop worker(num:4) vector_length(32) copyout(data[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 7.0f;
    }
    verify_array(data, 10, 7.0f);

    // Case 7: fully partitioned
    printf("Testing Case 7: fully partitioned\n");
    #pragma acc parallel loop gang(num:4) worker(num:2) vector_length(16) copyout(data[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 8.0f;
    }
    verify_array(data, 10, 8.0f);

    // Additional test using kernels construct for more coverage
    printf("Testing with kernels construct...\n");
    
    // Test gang redundant with kernels
    #pragma acc kernels loop gang(redundant) copyout(data[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 9.0f;
    }
    
    // Test gang partitioned with kernels
    #pragma acc kernels loop gang(num:8) copyout(data[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 10.0f;
    }
    
    // Test fully partitioned with kernels
    #pragma acc kernels loop gang(num:2) worker(num:4) vector_length(32) copyout(data[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = 11.0f;
    }

    // Test nested parallelism to potentially trigger different partition types
    printf("Testing nested parallelism...\n");
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16) copyout(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < ARRAY_SIZE/4; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < 4; j++) {
                int idx = i*4 + j;
                if (idx < ARRAY_SIZE) {
                    data[idx] = 12.0f;
                }
            }
        }
    }

    printf("All OpenACC partition tests completed.\n");
    
    free(data);
    return 0;
}
