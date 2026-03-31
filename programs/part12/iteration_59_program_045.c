/* test_neuter_broadcast.c - Comprehensive test for GCC omp-oacc-neuter-broadcast pass */
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
    int arr1[N];                         /* 1D array */
    int arr2[N][M];                      /* 2D array */
    int arr3[N][M][P];                   /* 3D array - complex partitioning */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1[i] = i;
        for (int j = 0; j < M; j++) {
            arr2[i][j] = i + j;
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = i + j + k;
            }
        }
    }
    
    #pragma acc parallel loop copy(result[0:N]) \
        copyin(arr1[0:N], arr2[0:N][0:M], arr3[0:N][0:M][0:P]) \
        private(scalar_private) firstprivate(scalar_redundant)
    for (i = 0; i < N; i++) {
        scalar_private = i % 10;
        
        /* Nested loops to create complex data flow */
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                /* Conditional operations */
                if (arr3[i][j][k] % 2 == 0) {
                    arr2[i][j] += scalar_redundant;
                } else {
                    arr2[i][j] -= scalar_private;
                }
            }
        }
        
        /* Reduction-like pattern */
        int sum = 0;
        for (int j = 0; j < M; j++) {
            sum += arr2[i][j];
        }
        result[i] = sum + arr1[i];
    }
}

/* Pattern B: Multi-dimensional arrays with complex access patterns */
void test_pattern_b(int *output) {
    int matrix1[M][M];
    int matrix2[M][M];
    int vector[M];
    
    /* Initialize */
    for (int i = 0; i < M; i++) {
        vector[i] = i;
        for (int j = 0; j < M; j++) {
            matrix1[i][j] = i * M + j;
            matrix2[i][j] = 0;
        }
    }
    
    #pragma acc parallel loop collapse(2) \
        copy(matrix2[0:M][0:M]) copyin(matrix1[0:M][0:M], vector[0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            /* Access with different strides */
            int idx = (i + j) % M;
            matrix2[i][j] = matrix1[i][idx] * vector[j];
            
            /* Nested conditional */
            if (i > j) {
                matrix2[i][j] += matrix1[j][i];
            } else if (i < j) {
                matrix2[i][j] -= vector[i];
            }
        }
    }
    
    /* Process results */
    #pragma acc parallel loop copy(output[0:M]) \
        copyin(matrix2[0:M][0:M])
    for (int i = 0; i < M; i++) {
        output[i] = 0;
        for (int j = 0; j < M; j++) {
            output[i] += matrix2[i][j];
        }
    }
}

/* Pattern C: Pointers and dynamic memory */
void test_pattern_c(int *final_result) {
    int size = N * M;
    int *dynamic_arr1 = (int *)malloc(size * sizeof(int));
    int *dynamic_arr2 = (int *)malloc(size * sizeof(int));
    
    /* Initialize dynamic arrays */
    for (int i = 0; i < size; i++) {
        dynamic_arr1[i] = i % 100;
        dynamic_arr2[i] = 0;
    }
    
    /* Map dynamic arrays to device */
    #pragma acc enter data copyin(dynamic_arr1[0:size])
    #pragma acc enter data create(dynamic_arr2[0:size])
    
    int chunk_size = 128;
    #pragma acc parallel loop gang vector \
        present(dynamic_arr1[0:size], dynamic_arr2[0:size]) \
        copy(final_result[0:chunk_size])
    for (int i = 0; i < size; i += chunk_size) {
        int local_sum = 0;
        int end = i + chunk_size;
        if (end > size) end = size;
        
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int j = i; j < end; j++) {
            dynamic_arr2[j] = dynamic_arr1[j] * 2;
            if (j % 3 == 0) {
                dynamic_arr2[j] += 1;
            }
            local_sum += dynamic_arr2[j];
        }
        
        if (i / chunk_size < chunk_size) {
            final_result[i / chunk_size] = local_sum;
        }
    }
    
    #pragma acc exit data copyout(dynamic_arr2[0:size])
    #pragma acc exit data delete(dynamic_arr1[0:size])
    
    free(dynamic_arr1);
    free(dynamic_arr2);
}

/* Pattern D: Structs and mixed data types */
typedef struct {
    int id;
    float values[4];
    double weight;
    char flags[8];
} ComplexData;

void test_pattern_d(int *checksum) {
    ComplexData data_array[N];
    
    /* Initialize struct array */
    for (int i = 0; i < N; i++) {
        data_array[i].id = i;
        data_array[i].weight = i * 0.1;
        for (int j = 0; j < 4; j++) {
            data_array[i].values[j] = i + j * 0.5f;
        }
        for (int j = 0; j < 8; j++) {
            data_array[i].flags[j] = (i + j) % 2;
        }
    }
    
    float float_results[N];
    int int_results[N];
    
    #pragma acc parallel loop copy(data_array[0:N]) \
        copyout(float_results[0:N], int_results[0:N])
    for (int i = 0; i < N; i++) {
        /* Access different struct members */
        float sum = 0.0f;
        for (int j = 0; j < 4; j++) {
            sum += data_array[i].values[j];
        }
        float_results[i] = sum * data_array[i].weight;
        
        int flag_sum = 0;
        for (int j = 0; j < 8; j++) {
            flag_sum += data_array[i].flags[j];
        }
        int_results[i] = data_array[i].id + flag_sum;
        
        /* Modify struct in place */
        data_array[i].weight += 0.01;
    }
    
    /* Final reduction */
    #pragma acc parallel loop reduction(+:*checksum) \
        copyin(int_results[0:N])
    for (int i = 0; i < N; i++) {
        *checksum += int_results[i];
    }
}

/* Main test driver */
int main() {
    int result_a[N];
    int result_b[M];
    int result_c[128];
    int checksum_d = 0;
    
    printf("Starting comprehensive neuter-broadcast test...\n");
    
    /* Execute all patterns to trigger different partitioning states */
    test_pattern_a(result_a);
    test_pattern_b(result_b);
    test_pattern_c(result_c);
    test_pattern_d(&checksum_d);
    
    /* Verify results are non-zero (simple validation) */
    int total_sum = 0;
    for (int i = 0; i < N; i++) total_sum += result_a[i];
    for (int i = 0; i < M; i++) total_sum += result_b[i];
    for (int i = 0; i < 128; i++) total_sum += result_c[i];
    total_sum += checksum_d;
    
    printf("Total sum: %d\n", total_sum);
    
    if (total_sum != 0) {
        printf("Test PASSED - All patterns executed successfully\n");
        return 0;
    } else {
        printf("Test FAILED - Unexpected zero result\n");
        return 1;
    }
}
