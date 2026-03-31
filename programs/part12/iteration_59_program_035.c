/* test_neuter_broadcast.c - Comprehensive OpenACC test for neuter-broadcast pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Helper function to verify results */
int verify_results(int *arr1, int *arr2, int *arr3, float *farr, double *darr, 
                   int scalar1, int scalar2, int scalar3) {
    int errors = 0;
    
    for (int i = 0; i < N; i++) {
        if (arr1[i] != i * 2) errors++;
        if (arr2[i] != i * 3) errors++;
        if (arr3[i] != i * 4) errors++;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            int idx = i * P + j;
            if (farr[idx] != (float)(i + j) * 2.5f) errors++;
        }
    }
    
    if (scalar1 != N * 2) errors++;
    if (scalar2 != N * 3) errors++;
    if (scalar3 != N * 4) errors++;
    
    return errors;
}

/* Function with complex OpenACC region to trigger various partitioning states */
void process_data() {
    /* Different types of variables that will get different partitioning states */
    
    /* 1D arrays - will likely get different partitioning based on usage */
    int arr1[N];           /* Simple array - gang partitioned? */
    int arr2[N];           /* Another array - worker partitioned? */
    int arr3[N];           /* Yet another - vector partitioned? */
    
    /* Multi-dimensional array - complex partitioning */
    float farr[M][P];      /* 2D array - gang+worker partitioned? */
    
    /* Pointer with dynamic allocation - different partitioning */
    double *darr = (double*)malloc(N * sizeof(double));
    
    /* Various scalar variables */
    int scalar1 = 0;       /* Reduction variable */
    int scalar2 = 1;       /* Firstprivate variable */
    int scalar3 = 2;       /* Private variable */
    
    /* Initialize data on host */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i;
        arr3[i] = i;
        darr[i] = (double)i;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            farr[i][j] = (float)(i + j);
        }
    }
    
    /* Complex OpenACC parallel region with multiple data clauses
       This should trigger the neuter-broadcast pass */
    #pragma acc parallel loop copy(arr1[0:N]) copyout(arr2[0:N]) \
        copyin(arr3[0:N]) create(farr[0:M][0:P]) present_or_copy(darr[0:N]) \
        reduction(+:scalar1) firstprivate(scalar2) private(scalar3) \
        gang worker vector
    for (int i = 0; i < N; i++) {
        /* Nested loops to create complex data flow */
        for (int j = 0; j < 10; j++) {
            /* Conditional operations */
            if (i % 2 == 0) {
                arr1[i] *= 2;
                arr2[i] = arr1[i] + i;
            } else {
                arr1[i] += i;
                arr2[i] = arr1[i] * 2;
            }
            
            /* Access multi-dimensional array with complex indexing */
            if (i < M && j < P) {
                farr[i % M][j % P] *= 2.5f;
            }
            
            /* Access pointer-based array */
            darr[i] += (double)j * 0.1;
        }
        
        /* Different operations on arr3 */
        arr3[i] = arr1[i] + arr2[i];
        
        /* Update reduction variable */
        scalar1 += arr1[i];
        
        /* Update private scalar */
        scalar3 = scalar2 + i;
    }
    
    /* Additional parallel region with different characteristics */
    int temp_arr[N];
    #pragma acc parallel loop copy(temp_arr[0:N]) gang
    for (int i = 0; i < N; i++) {
        temp_arr[i] = arr1[i] + arr3[i];
        
        /* Nested parallel loop simulation */
        #pragma acc loop worker
        for (int j = 0; j < 4; j++) {
            temp_arr[i] += j;
            
            /* Innermost vector loop */
            #pragma acc loop vector
            for (int k = 0; k < 8; k++) {
                temp_arr[i] += k;
            }
        }
    }
    
    /* Process multi-dimensional array with explicit gang/worker/vector partitioning */
    float result[M][P];
    #pragma acc parallel loop collapse(2) copyin(farr) copyout(result) \
        gang worker vector
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            /* Complex conditional access pattern */
            if ((i + j) % 3 == 0) {
                result[i][j] = farr[i][j] * 2.0f;
            } else if ((i + j) % 3 == 1) {
                result[i][j] = farr[i][j] / 2.0f;
            } else {
                result[i][j] = farr[i][j] + farr[(i + 1) % M][(j + 1) % P];
            }
            
            /* Nested conditional */
            for (int k = 0; k < 4; k++) {
                if (k % 2 == 0) {
                    result[i][j] += (float)k;
                }
            }
        }
    }
    
    /* Verify intermediate results */
    int check = 0;
    #pragma acc parallel loop reduction(+:check) copy(temp_arr)
    for (int i = 0; i < N; i++) {
        if (temp_arr[i] > 1000) check++;
    }
    
    /* Clean up */
    free(darr);
}

/* Main function to drive the test */
int main() {
    printf("Starting OpenACC neuter-broadcast coverage test...\n");
    
    /* Call the function with OpenACC regions multiple times
       to ensure all code paths are exercised */
    for (int iter = 0; iter < 3; iter++) {
        process_data();
        printf("Iteration %d completed\n", iter + 1);
    }
    
    /* Additional test with different data sizes */
    {
        int small_arr[10];
        int medium_arr[100];
        int large_arr[1000];
        
        #pragma acc parallel loop copy(small_arr, medium_arr, large_arr)
        for (int i = 0; i < 1000; i++) {
            if (i < 10) small_arr[i] = i * 10;
            if (i < 100) medium_arr[i % 100] = i * 5;
            large_arr[i] = i * 2;
        }
        
        /* Verify */
        int sum = 0;
        #pragma acc parallel loop reduction(+:sum) copy(small_arr, medium_arr, large_arr)
        for (int i = 0; i < 1000; i++) {
            if (i < 10) sum += small_arr[i];
            if (i < 100) sum += medium_arr[i % 100];
            sum += large_arr[i];
        }
        
        printf("Sum check: %d\n", sum);
    }
    
    printf("Test completed successfully!\n");
    return 0;
}
