/* Test program to cover all partition type cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024

/* Helper function to ensure computations aren't optimized away */
static void verify_result(int *arr, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    if (sum != expected_sum) {
        fprintf(stderr, "Verification failed: got %d, expected %d\n", sum, expected_sum);
    }
}

int main() {
    int *data = (int*)malloc(N * sizeof(int));
    if (!data) return 1;
    
    /* Case 0: gang redundant */
    printf("Testing case 0: gang redundant\n");
    #pragma acc parallel gang(redundant) copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = 1;
        }
    }
    verify_result(data, N);
    
    /* Case 1: gang partitioned */
    printf("Testing case 1: gang partitioned\n");
    #pragma acc parallel gang(num:32) copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = 2;
        }
    }
    verify_result(data, N * 2);
    
    /* Case 2: worker partitioned */
    printf("Testing case 2: worker partitioned\n");
    #pragma acc parallel worker(num:4) copyout(data[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            data[i] = 3;
        }
    }
    verify_result(data, N * 3);
    
    /* Case 3: gang+worker partitioned */
    printf("Testing case 3: gang+worker partitioned\n");
    #pragma acc parallel gang(num:16) worker(num:8) copyout(data[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            data[i] = 4;
        }
    }
    verify_result(data, N * 4);
    
    /* Case 4: vector partitioned */
    printf("Testing case 4: vector partitioned\n");
    #pragma acc parallel vector_length(128) copyout(data[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            data[i] = 5;
        }
    }
    verify_result(data, N * 5);
    
    /* Case 5: gang+vector partitioned */
    printf("Testing case 5: gang+vector partitioned\n");
    #pragma acc parallel gang(num:8) vector_length(64) copyout(data[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            data[i] = 6;
        }
    }
    verify_result(data, N * 6);
    
    /* Case 6: worker+vector partitioned */
    printf("Testing case 6: worker+vector partitioned\n");
    #pragma acc parallel worker(num:4) vector_length(32) copyout(data[0:N])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            data[i] = 7;
        }
    }
    verify_result(data, N * 7);
    
    /* Case 7: fully partitioned */
    printf("Testing case 7: fully partitioned\n");
    #pragma acc parallel gang(num:4) worker(num:2) vector_length(16) copyout(data[0:N])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            data[i] = 8;
        }
    }
    verify_result(data, N * 8);
    
    /* Additional test using kernels construct which may trigger different paths */
    printf("Testing kernels construct variations\n");
    
    /* Kernels with gang partitioning */
    #pragma acc kernels gang(num:8) copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = 9;
        }
    }
    verify_result(data, N * 9);
    
    /* Kernels with worker partitioning */
    #pragma acc kernels worker(num:4) copyout(data[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            data[i] = 10;
        }
    }
    verify_result(data, N * 10);
    
    /* Kernels with vector partitioning */
    #pragma acc kernels vector_length(64) copyout(data[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            data[i] = 11;
        }
    }
    verify_result(data, N * 11);
    
    /* Test nested parallelism which might trigger different partition types */
    printf("Testing nested parallelism\n");
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) copyout(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N/4; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < 4; j++) {
                int idx = i*4 + j;
                if (idx < N) data[idx] = 12;
            }
        }
    }
    verify_result(data, N * 12);
    
    free(data);
    
    /* For the default case (<illegal>), we cannot directly call the internal function
       from user code. However, the runtime might use invalid codes internally in error
       conditions. We can try to trigger error conditions by using invalid combinations */
    printf("Testing potential error conditions\n");
    
    /* Try using invalid clause combinations that might cause internal errors */
    int small_data[10];
    
    /* This might cause internal confusion about partition type */
    #pragma acc parallel copyout(small_data[0:10])
    {
        /* No explicit partitioning - let runtime decide */
        #pragma acc loop
        for (int i = 0; i < 10; i++) {
            small_data[i] = i;
        }
    }
    
    /* Test with async clause which might use different internal paths */
    #pragma acc parallel async(1) copyout(small_data[0:10])
    {
        #pragma acc loop
        for (int i = 0; i < 10; i++) {
            small_data[i] = i * 2;
        }
    }
    #pragma acc wait(1)
    
    printf("All tests completed\n");
    return 0;
}
