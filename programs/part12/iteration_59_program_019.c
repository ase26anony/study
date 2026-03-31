/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * 
 * This program uses OpenACC constructs to create variables with different
 * partitioning attributes, aiming to trigger all cases in the partitioning
 * state switch statement (cases 0-7).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_pattern_a(int *result) {
    int i;
    int scalar_redundant = 42;           /* Likely gang redundant (0) */
    int scalar_private;                  /* Worker/vector partitioned */
    int arr1d[N];                        /* 1D array */
    int arr2d[M][N];                     /* 2D array */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
    }
    for (int j = 0; j < M; j++) {
        for (i = 0; i < N; i++) {
            arr2d[j][i] = j * N + i;
        }
    }
    
    #pragma acc parallel loop copy(result[0:N]) copyin(scalar_redundant, arr1d, arr2d) \
        private(scalar_private) gang worker vector
    for (i = 0; i < N; i++) {
        int local_var = 0;               /* Private to each thread */
        
        /* Complex nested loops to create varied data flow */
        for (int j = 0; j < M; j++) {
            /* Conditional operations */
            if (j % 2 == 0) {
                scalar_private = arr1d[i] + arr2d[j][i];
            } else {
                scalar_private = arr1d[i] - arr2d[j][i];
            }
            
            /* Multi-dimensional access pattern */
            for (int k = 0; k < P; k++) {
                if (k % 3 == 0) {
                    local_var += scalar_redundant * scalar_private;
                } else if (k % 3 == 1) {
                    local_var -= scalar_redundant / (scalar_private + 1);
                } else {
                    local_var ^= scalar_private;
                }
            }
        }
        
        /* Reduction-like operation */
        result[i] = local_var % 256;
    }
}

/* Pattern B: Multi-dimensional arrays with complex access patterns */
void test_pattern_b(int *output) {
    int arr3d[P][M][N];                  /* 3D array - complex partitioning */
    int partial_sums[M][N];
    
    /* Initialize 3D array */
    for (int k = 0; k < P; k++) {
        for (int j = 0; j < M; j++) {
            for (int i = 0; i < N; i++) {
                arr3d[k][j][i] = k * M * N + j * N + i;
            }
        }
    }
    
    #pragma acc parallel loop collapse(2) copyout(output[0:N]) \
        copyin(arr3d) create(partial_sums) gang worker vector
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            int sum = 0;
            
            /* Access 3D array with varying indices across dimensions */
            for (int k = 0; k < P; k++) {
                /* Different access patterns for different partitioning */
                if (k % 4 == 0) {
                    sum += arr3d[k][j][i];           /* Gang+worker partitioned? */
                } else if (k % 4 == 1) {
                    sum += arr3d[k][(j + 1) % M][i]; /* Worker partitioned? */
                } else if (k % 4 == 2) {
                    sum += arr3d[k][j][(i + 1) % N]; /* Vector partitioned? */
                } else {
                    sum += arr3d[(k + 1) % P][j][i]; /* Gang partitioned? */
                }
            }
            
            partial_sums[j][i] = sum;
            
            /* Final computation with conditional */
            if ((j + i) % 2 == 0) {
                output[i] += partial_sums[j][i] / (j + 2);
            } else {
                output[i] -= partial_sums[j][i] % (i + 2);
            }
        }
    }
}

/* Pattern C: Pointers and dynamic memory */
void test_pattern_c(int *final_result) {
    int *dynamic_arr1;
    int *dynamic_arr2;
    int static_arr[N];
    int *ptr_var;
    
    /* Dynamic allocation */
    dynamic_arr1 = (int *)malloc(N * M * sizeof(int));
    dynamic_arr2 = (int *)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < N * M; i++) {
        dynamic_arr1[i] = i * 2;
    }
    for (int i = 0; i < N; i++) {
        dynamic_arr2[i] = i * 3;
        static_arr[i] = i * 5;
    }
    
    /* Enter data region for dynamic arrays */
    #pragma acc enter data copyin(dynamic_arr1[0:N*M], dynamic_arr2[0:N])
    
    #pragma acc parallel loop copy(final_result[0:N]) copyin(static_arr) \
        present(dynamic_arr1, dynamic_arr2) private(ptr_var) gang worker vector
    for (int i = 0; i < N; i++) {
        int temp = 0;
        
        /* Switch between different pointers */
        if (i % 3 == 0) {
            ptr_var = dynamic_arr1 + i * M;
            for (int j = 0; j < M; j++) {
                temp += ptr_var[j];
            }
        } else if (i % 3 == 1) {
            ptr_var = dynamic_arr2;
            temp = ptr_var[i] * static_arr[i];
        } else {
            ptr_var = &static_arr[0];
            for (int j = 0; j < 10; j++) {
                temp += ptr_var[(i + j) % N];
            }
        }
        
        /* Complex computation with nested conditionals */
        for (int k = 0; k < 8; k++) {
            if (temp % (k + 2) == 0) {
                final_result[i] += temp / (k + 2);
            } else {
                final_result[i] -= temp % (k + 2);
            }
        }
    }
    
    #pragma acc exit data delete(dynamic_arr1, dynamic_arr2)
    free(dynamic_arr1);
    free(dynamic_arr2);
}

/* Pattern D: Struct/aggregate data types */
typedef struct {
    int x;
    int y;
    float z;
    double w;
    int arr[4];
} ComplexData;

void test_pattern_d(int *verification) {
    ComplexData data_array[N];
    int reduction_var = 0;
    
    /* Initialize struct array */
    for (int i = 0; i < N; i++) {
        data_array[i].x = i;
        data_array[i].y = i * 2;
        data_array[i].z = i * 0.5f;
        data_array[i].w = i * 0.25;
        for (int j = 0; j < 4; j++) {
            data_array[i].arr[j] = i * 10 + j;
        }
    }
    
    #pragma acc parallel loop copyout(verification[0:N]) copyin(data_array) \
        reduction(+:reduction_var) gang worker vector
    for (int i = 0; i < N; i++) {
        /* Access different struct members with different patterns */
        int local_sum = 0;
        
        /* Access .x (likely scalar) */
        local_sum += data_array[i].x;
        
        /* Access .arr with loop (array within struct) */
        for (int j = 0; j < 4; j++) {
            if (j % 2 == 0) {
                local_sum += data_array[i].arr[j];
            } else {
                local_sum -= data_array[i].arr[j];
            }
        }
        
        /* Conditional access to .y and .z */
        if (i % 2 == 0) {
            local_sum *= (int)data_array[i].z;
        } else {
            local_sum /= (data_array[i].y + 1);
        }
        
        /* Nested loop with struct member access */
        for (int k = 0; k < 8; k++) {
            if (k < 4) {
                local_sum ^= data_array[(i + k) % N].arr[k];
            } else {
                local_sum |= data_array[(i + k) % N].x;
            }
        }
        
        verification[i] = local_sum;
        reduction_var += local_sum;
    }
    
    /* Use reduction result */
    verification[0] += reduction_var % 1000;
}

/* Main function that runs all patterns */
int main() {
    int *result_a = (int *)calloc(N, sizeof(int));
    int *result_b = (int *)calloc(N, sizeof(int));
    int *result_c = (int *)calloc(N, sizeof(int));
    int *result_d = (int *)calloc(N, sizeof(int));
    
    printf("Starting comprehensive neuter-broadcast test...\n");
    
    /* Execute all test patterns */
    test_pattern_a(result_a);
    test_pattern_b(result_b);
    test_pattern_c(result_c);
    test_pattern_d(result_d);
    
    /* Verification - simple checksum */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += result_a[i] + result_b[i] + result_c[i] + result_d[i];
        checksum %= 1000000;
    }
    
    printf("Test completed. Checksum: %d\n", checksum);
    
    /* Basic validation */
    if (checksum != 0) {
        printf("Validation passed (non-zero checksum).\n");
    }
    
    free(result_a);
    free(result_b);
    free(result_c);
    free(result_d);
    
    return 0;
}
