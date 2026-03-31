/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * 
 * This program uses OpenACC constructs to create variables with different
 * partitioning characteristics, aiming to trigger all cases (0-7) in the
 * partitioning state switch statement.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Pattern A: Different scalar and array types with various data clauses */
void test_pattern_a(int *result) {
    int i;
    int scalar_private;          /* Likely gang redundant (0) */
    int scalar_firstprivate = 5; /* Likely gang redundant (0) */
    int scalar_reduction = 0;    /* Special handling */
    
    int arr1d[N];                /* 1D array - various partitioning possible */
    int arr2d[M][M];             /* 2D array - more complex partitioning */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i % 100;
    }
    
    for (int j = 0; j < M; j++) {
        for (int k = 0; k < M; k++) {
            arr2d[j][k] = j * M + k;
        }
    }
    
    /* OpenACC parallel region with multiple data clauses */
    #pragma acc parallel loop copyout(arr1d[0:N]) \
        copyin(arr2d[0:M][0:M]) private(scalar_private) \
        firstprivate(scalar_firstprivate) reduction(+:scalar_reduction)
    for (i = 0; i < N; i++) {
        scalar_private = i % 10;
        
        /* Nested loop accessing 2D array - creates complex data flow */
        for (int j = 0; j < M; j++) {
            if (j < i % M) {
                arr1d[i] += arr2d[j][i % M];
                scalar_reduction += arr2d[j][i % M] % 5;
            }
        }
        
        /* Conditional operations */
        if (i % 3 == 0) {
            arr1d[i] *= scalar_firstprivate;
        } else if (i % 3 == 1) {
            arr1d[i] += scalar_private;
        }
    }
    
    /* Store result for verification */
    *result = scalar_reduction;
}

/* Pattern B: Multi-dimensional arrays with complex access patterns */
void test_pattern_b(int *checksum) {
    int arr3d[P][M][M];  /* 3D array - high likelihood of complex partitioning */
    int sum = 0;
    
    /* Initialize 3D array */
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < M; k++) {
                arr3d[i][j][k] = (i * 1000) + (j * 10) + k;
            }
        }
    }
    
    /* Complex parallel region with nested loops */
    #pragma acc parallel loop collapse(2) copyin(arr3d[0:P][0:M][0:M]) \
        copyout(sum) reduction(+:sum)
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            int local_sum = 0;
            
            /* Vector-like inner loop */
            for (int k = 0; k < M; k++) {
                /* Access with different strides and patterns */
                if ((i + j + k) % 2 == 0) {
                    local_sum += arr3d[i][j][k];
                } else {
                    local_sum -= arr3d[i][j][k] / 2;
                }
                
                /* Cross-dimensional access */
                if (k % 4 == 0 && i > 0) {
                    local_sum += arr3d[i-1][j][k] % 100;
                }
            }
            
            sum += local_sum;
        }
    }
    
    *checksum = sum;
}

/* Pattern C: Pointer-based dynamic memory */
void test_pattern_c(int *final_result) {
    int *dynamic_arr;
    int *host_arr;
    int size = N * M;
    
    /* Allocate and initialize host memory */
    host_arr = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        host_arr[i] = i % 256;
    }
    
    /* Allocate device memory */
    dynamic_arr = (int *)malloc(size * sizeof(int));
    
    /* OpenACC data region with dynamic arrays */
    #pragma acc data copyin(host_arr[0:size]) copyout(dynamic_arr[0:size])
    {
        #pragma acc parallel loop
        for (int i = 0; i < size; i++) {
            /* Complex pointer arithmetic and conditional access */
            int idx = i;
            int val = host_arr[idx];
            
            /* Nested conditional operations */
            for (int stride = 1; stride < 8; stride *= 2) {
                if (i % stride == 0 && i + stride < size) {
                    val += host_arr[i + stride] / (stride + 1);
                }
            }
            
            /* Different computation based on position */
            if (i < size / 2) {
                dynamic_arr[i] = val * 2;
            } else {
                dynamic_arr[i] = val / 2;
            }
        }
    }
    
    /* Verify results */
    int total = 0;
    for (int i = 0; i < size; i++) {
        total += dynamic_arr[i];
    }
    
    free(host_arr);
    free(dynamic_arr);
    
    *final_result = total % 10000;
}

/* Pattern D: Struct-based data (C structs) */
typedef struct {
    int x;
    int y;
    float z;
    double w;
} ComplexData;

void test_pattern_d(int *struct_result) {
    ComplexData data_arr[N];
    int temp_results[M];
    int final_sum = 0;
    
    /* Initialize struct array */
    for (int i = 0; i < N; i++) {
        data_arr[i].x = i;
        data_arr[i].y = i * 2;
        data_arr[i].z = i * 0.5f;
        data_arr[i].w = i * 0.25;
    }
    
    /* Initialize temp array */
    for (int i = 0; i < M; i++) {
        temp_results[i] = 0;
    }
    
    /* Complex OpenACC region with struct access */
    #pragma acc parallel loop copyin(data_arr[0:N]) \
        copyout(temp_results[0:M]) reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        int gang_idx = i / (N/M);
        if (gang_idx < M) {
            /* Access different struct members with different patterns */
            temp_results[gang_idx] += data_arr[i].x;
            
            if (i % 3 == 0) {
                temp_results[gang_idx] += (int)(data_arr[i].z * 2);
            }
            
            /* Nested loop with conditional */
            for (int j = 0; j < 4; j++) {
                if ((i + j) % 5 == 0) {
                    temp_results[gang_idx] -= j;
                }
            }
        }
        
        /* Reduction on struct member */
        final_sum += data_arr[i].y;
    }
    
    /* Post-process on host */
    int total = final_sum;
    for (int i = 0; i < M; i++) {
        total += temp_results[i];
    }
    
    *struct_result = total;
}

/* Combined test with all patterns */
void comprehensive_test() {
    int result_a = 0, result_b = 0, result_c = 0, result_d = 0;
    int validation_sum = 0;
    
    printf("Running comprehensive neuter-broadcast test...\n");
    
    /* Execute all test patterns */
    test_pattern_a(&result_a);
    validation_sum = (validation_sum + result_a) % 1000;
    
    test_pattern_b(&result_b);
    validation_sum = (validation_sum + result_b) % 1000;
    
    test_pattern_c(&result_c);
    validation_sum = (validation_sum + result_c) % 1000;
    
    test_pattern_d(&result_d);
    validation_sum = (validation_sum + result_d) % 1000;
    
    /* Final verification */
    int expected_pattern = (result_a + result_b + result_c + result_d) % 1000;
    
    if (validation_sum == expected_pattern) {
        printf("Test PASSED: All patterns executed correctly\n");
        printf("Results: A=%d, B=%d, C=%d, D=%d, Total=%d\n", 
               result_a, result_b, result_c, result_d, validation_sum);
    } else {
        printf("Test FAILED: validation_sum=%d, expected=%d\n", 
               validation_sum, expected_pattern);
    }
}

int main() {
    comprehensive_test();
    return 0;
}
