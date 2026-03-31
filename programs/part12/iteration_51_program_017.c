/* Test program to cover partition type string conversion in GCC OpenACC runtime */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N 1024

/* Function to verify computations (prevents optimization) */
void verify_array(float *arr, int size, float expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            fprintf(stderr, "Error: arr[%d] = %f, expected %f\n", i, arr[i], expected);
            exit(1);
        }
    }
}

int main() {
    float *data = (float*)malloc(N * sizeof(float));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    /* Case 0: gang redundant */
    #pragma acc parallel gang(redundant) copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = 1.0f;
        }
    }
    verify_array(data, N, 1.0f);

    /* Case 1: gang partitioned */
    #pragma acc parallel gang(num:32) copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = 2.0f;
        }
    }
    verify_array(data, N, 2.0f);

    /* Case 2: worker partitioned */
    #pragma acc parallel worker(num:4) copyout(data[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            data[i] = 3.0f;
        }
    }
    verify_array(data, N, 3.0f);

    /* Case 3: gang+worker partitioned */
    #pragma acc parallel gang(num:16) worker(num:8) copyout(data[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            data[i] = 4.0f;
        }
    }
    verify_array(data, N, 4.0f);

    /* Case 4: vector partitioned */
    #pragma acc parallel vector_length(128) copyout(data[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            data[i] = 5.0f;
        }
    }
    verify_array(data, N, 5.0f);

    /* Case 5: gang+vector partitioned */
    #pragma acc parallel gang(num:8) vector_length(64) copyout(data[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            data[i] = 6.0f;
        }
    }
    verify_array(data, N, 6.0f);

    /* Case 6: worker+vector partitioned */
    #pragma acc parallel worker(num:4) vector_length(32) copyout(data[0:N])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            data[i] = 7.0f;
        }
    }
    verify_array(data, N, 7.0f);

    /* Case 7: fully partitioned */
    #pragma acc parallel gang(num:4) worker(num:2) vector_length(16) copyout(data[0:N])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            data[i] = 8.0f;
        }
    }
    verify_array(data, N, 8.0f);

    /* Attempt to trigger default case (illegal partition type) */
    /* This is tricky as we need to cause the runtime to call the function
       with an out-of-range value. We'll try some unusual combinations that
       might trigger internal error paths. */
    
    /* Combination that might confuse the runtime */
    #pragma acc parallel copyout(data[0:N])
    {
        /* No explicit partitioning - let runtime choose */
        #pragma acc loop
        for (int i = 0; i < N; i++) {
            data[i] = 9.0f;
        }
    }
    verify_array(data, N, 9.0f);

    /* Another attempt: nested parallelism might trigger unusual states */
    #pragma acc parallel copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N/2; i++) {
            #pragma acc loop worker
            for (int j = 0; j < 2; j++) {
                data[i*2 + j] = 10.0f;
            }
        }
    }
    verify_array(data, N, 10.0f);

    /* Try kernels construct with various partitionings */
    #pragma acc kernels copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = 11.0f;
        }
    }
    verify_array(data, N, 11.0f);

    #pragma acc kernels copyout(data[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            data[i] = 12.0f;
        }
    }
    verify_array(data, N, 12.0f);

    free(data);
    
    printf("All OpenACC partition tests completed successfully\n");
    return 0;
}
