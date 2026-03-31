/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in GCC's omp-oacc-neuter-broadcast pass
 * Compile with: gcc -O2 -fopenacc -foffload=disable -fdump-tree-all -fprofile-arcs -ftest-coverage test_neuter_broadcast.c -o test_neuter_broadcast
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern A: Various scalar and array variables with different data clauses */
void test_pattern_a(int *result) {
    int scalar_private;          /* Likely gang redundant (0) */
    int scalar_firstprivate = 42; /* Likely gang redundant (0) */
    int scalar_reduction = 0;    /* Reduction variable */
    
    int arr1d[N];                /* 1D array */
    int arr2d[M][M];             /* 2D array */
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = (i * M + j) % 100;
        }
    }
    
    #pragma acc parallel loop copy(arr1d[0:N]) copyin(arr2d) \
        private(scalar_private) firstprivate(scalar_firstprivate) \
        reduction(+:scalar_reduction)
    for (int i = 0; i < N; i++) {
        scalar_private = i;
        int temp = arr1d[i] + scalar_private + scalar_firstprivate;
        
        /* Nested loop accessing 2D array */
        for (int j = 0; j < M; j++) {
            if (j % 2 == 0) {
                temp += arr2d[i % M][j];
            } else {
                temp -= arr2d[j][i % M];
            }
        }
        
        arr1d[i] = temp % 1000;
        scalar_reduction += temp;
    }
    
    *result = scalar_reduction;
}

/* Pattern B: Multi-dimensional arrays with complex access patterns */
void test_pattern_b(int *result) {
    int arr3d[P][M][M];  /* 3D array - complex partitioning */
    int arr2d_slice[M][M];
    
    /* Initialize 3D array */
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < M; k++) {
                arr3d[i][j][k] = (i * M * M + j * M + k) % 256;
            }
        }
    }
    
    #pragma acc parallel loop collapse(2) copyout(arr2d_slice) copyin(arr3d)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            int sum = 0;
            
            /* Access 3D array with varying indices across dimensions */
            for (int k = 0; k < P; k++) {
                /* Complex conditional access pattern */
                if ((i + j + k) % 3 == 0) {
                    sum += arr3d[k][i][j];
                } else if ((i + j + k) % 3 == 1) {
                    sum += arr3d[i % P][k % M][j];
                } else {
                    sum += arr3d[j % P][i][k % M];
                }
            }
            
            arr2d_slice[i][j] = sum % 1000;
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            checksum = (checksum + arr2d_slice[i][j]) % 1000000;
        }
    }
    
    *result = checksum;
}

/* Pattern C: Pointer-based dynamic memory */
void test_pattern_c(int *result) {
    int *dynamic_arr1 = (int *)malloc(N * sizeof(int));
    int *dynamic_arr2 = (int *)malloc(N * sizeof(int));
    int *dynamic_arr3 = (int *)malloc(N * sizeof(int));
    
    /* Initialize dynamic arrays */
    for (int i = 0; i < N; i++) {
        dynamic_arr1[i] = i;
        dynamic_arr2[i] = i * 2;
        dynamic_arr3[i] = i * 3;
    }
    
    /* Enter data region for pointers */
    #pragma acc enter data copyin(dynamic_arr1[0:N], dynamic_arr2[0:N])
    #pragma acc enter data create(dynamic_arr3[0:N])
    
    int local_scalar = 100;
    
    #pragma acc parallel loop present(dynamic_arr1, dynamic_arr2, dynamic_arr3) \
        firstprivate(local_scalar)
    for (int i = 0; i < N; i++) {
        /* Complex pointer arithmetic and conditional access */
        int idx1 = i;
        int idx2 = (i * 7) % N;
        int idx3 = (i * 13) % N;
        
        if (i % 4 == 0) {
            dynamic_arr3[i] = dynamic_arr1[idx1] + dynamic_arr2[idx2] + local_scalar;
        } else if (i % 4 == 1) {
            dynamic_arr3[i] = dynamic_arr1[idx2] - dynamic_arr2[idx3];
        } else if (i % 4 == 2) {
            dynamic_arr3[i] = dynamic_arr1[idx3] * dynamic_arr2[idx1];
        } else {
            dynamic_arr3[i] = dynamic_arr1[idx1] + dynamic_arr2[idx2] - dynamic_arr2[idx3];
        }
        
        /* Nested loop with pointer access */
        for (int j = 0; j < 4; j++) {
            if ((i + j) % 2 == 0) {
                dynamic_arr3[i] += j;
            }
        }
    }
    
    #pragma acc exit data copyout(dynamic_arr3[0:N])
    
    /* Compute result */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum = (sum + dynamic_arr3[i]) % 1000000;
    }
    
    free(dynamic_arr1);
    free(dynamic_arr2);
    free(dynamic_arr3);
    
    *result = sum;
}

/* Pattern D: Mixed data types and complex control flow */
void test_pattern_d(int *result) {
    float float_arr[N];
    double double_arr[N];
    char char_arr[N];
    
    /* Initialize arrays with different data types */
    for (int i = 0; i < N; i++) {
        float_arr[i] = i * 0.1f;
        double_arr[i] = i * 0.01;
        char_arr[i] = (i % 26) + 'A';
    }
    
    int int_result = 0;
    float float_result = 0.0f;
    
    #pragma acc parallel loop copy(float_arr, double_arr, char_arr) \
        reduction(+:int_result, float_result)
    for (int i = 0; i < N; i++) {
        /* Complex conditional operations mixing types */
        if (i % 3 == 0) {
            float_arr[i] = float_arr[i] * 2.0f + (float)char_arr[i];
            int_result += (int)float_arr[i];
        } else if (i % 3 == 1) {
            double_arr[i] = double_arr[i] / 1.5 + (double)char_arr[i];
            float_result += (float)double_arr[i];
        } else {
            char_arr[i] = (char_arr[i] - 'A' + i) % 26 + 'A';
            int_result += char_arr[i];
        }
        
        /* Nested loops with mixed array access */
        for (int j = 0; j < 8; j++) {
            if ((i + j) % 5 == 0) {
                for (int k = 0; k < 4; k++) {
                    float_arr[(i + k) % N] += 0.1f;
                    double_arr[(i + j + k) % N] -= 0.01;
                }
            }
        }
    }
    
    /* Combine results */
    *result = int_result + (int)float_result;
}

/* Main test driver */
int main() {
    int result_a = 0, result_b = 0, result_c = 0, result_d = 0;
    int final_result = 0;
    
    printf("Starting comprehensive neuter-broadcast test...\n");
    
    /* Execute all test patterns */
    test_pattern_a(&result_a);
    printf("Pattern A result: %d\n", result_a);
    
    test_pattern_b(&result_b);
    printf("Pattern B result: %d\n", result_b);
    
    test_pattern_c(&result_c);
    printf("Pattern C result: %d\n", result_c);
    
    test_pattern_d(&result_d);
    printf("Pattern D result: %d\n", result_d);
    
    /* Combine results for final validation */
    final_result = (result_a + result_b + result_c + result_d) % 1000000;
    printf("Final combined result: %d\n", final_result);
    
    /* Simple validation - just check that results are non-zero */
    if (result_a != 0 && result_b != 0 && result_c != 0 && result_d != 0) {
        printf("All tests completed successfully!\n");
        return 0;
    } else {
        printf("Test validation failed!\n");
        return 1;
    }
}
