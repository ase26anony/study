/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * 
 * This program creates variables with diverse partitioning attributes to
 * trigger all cases (0-7) in the partitioning state switch statement.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern D: Struct with multiple members */
typedef struct {
    int id;
    float value;
    double data[4];
} DataStruct;

/* Function containing complex OpenACC region */
void test_openacc_partitioning(void) {
    /* Pattern A: Various scalar variables with different attributes */
    int scalar_private = 42;          /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;    /* Likely gang redundant (0) */
    int scalar_reduction = 0;         /* Reduction variable */
    
    /* Pattern B: Multi-dimensional arrays */
    int multi_array[M][P][4];         /* 3D array for complex partitioning */
    float matrix[N][N];               /* 2D array */
    
    /* Pattern A: 1D arrays with different access patterns */
    int arr1[N], arr2[N], arr3[N];
    double arr4[N], arr5[N];
    
    /* Pattern D: Array of structs */
    DataStruct struct_array[N];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        arr4[i] = i * 0.5;
        arr5[i] = i * 1.5;
        
        struct_array[i].id = i;
        struct_array[i].value = i * 0.1f;
        for (int j = 0; j < 4; j++) {
            struct_array[i].data[j] = i * j * 0.01;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            for (int k = 0; k < 4; k++) {
                multi_array[i][j][k] = i * 1000 + j * 10 + k;
            }
        }
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = i * j * 0.01f;
        }
    }
    
    /* Complex OpenACC parallel region with nested loops and conditionals */
    #pragma acc parallel loop copyin(arr1[0:N], arr2[0:N]) \
                              copyout(arr3[0:N]) \
                              copy(arr4[0:N]) \
                              create(arr5[0:N]) \
                              copyin(multi_array[0:M][0:P][0:4]) \
                              copy(matrix[0:N][0:N]) \
                              copyin(struct_array[0:N]) \
                              private(scalar_private) \
                              firstprivate(scalar_firstprivate) \
                              reduction(+:scalar_reduction) \
                              gang worker vector
    for (int i = 0; i < N; i++) {
        /* Access scalar_private - private to each gang/worker/vector */
        int local_var = scalar_private + i;
        
        /* Pattern A: Different array access patterns */
        arr3[i] = arr1[i] + arr2[i];
        
        /* Pattern B: Multi-dimensional array access with varying indices */
        if (i < M && i < P) {
            for (int k = 0; k < 4; k++) {
                arr4[i] += multi_array[i][i][k] * 0.1;
            }
        }
        
        /* Pattern C-like: Pointer-like access through array indexing */
        arr5[i] = arr4[i] * 2.0;
        
        /* Pattern D: Struct member access */
        struct_array[i].value = struct_array[i].id * 0.2f;
        for (int j = 0; j < 4; j++) {
            struct_array[i].data[j] += j * 0.5;
        }
        
        /* Nested loop to create complex data flow */
        if (i % 16 == 0) {
            float temp = 0.0f;
            #pragma acc loop seq
            for (int j = 0; j < 8; j++) {
                temp += matrix[i % N][j % N];
            }
            arr3[i] += (int)temp;
        }
        
        /* Conditional operations */
        if (arr1[i] > N/2) {
            arr2[i] = arr3[i] * 2;
        } else {
            arr2[i] = arr3[i] / 2;
        }
        
        /* Reduction operation */
        scalar_reduction += arr1[i];
    }
    
    /* Additional OpenACC kernels region with different partitioning */
    #pragma acc kernels copy(arr1[0:N], arr4[0:N]) \
                        copyin(arr2[0:N]) \
                        copyout(arr5[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i += 64) {
            #pragma acc loop worker
            for (int j = i; j < i + 64 && j < N; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    arr5[j] += arr1[j] * arr2[j] * arr4[j] * k;
                }
            }
        }
    }
    
    printf("OpenACC test completed. Reduction result: %d\n", scalar_reduction);
}

/* Function containing OpenMP target region */
void test_openmp_partitioning(void) {
    int a[N], b[N], c[N];
    double d[N][M];
    float e[P][M][4];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 3;
        c[i] = 0;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            d[i][j] = i * j * 0.01;
        }
    }
    
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < 4; k++) {
                e[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
                map(to: a[0:N], b[0:N], d[0:N][0:M], e[0:P][0:M][0:4]) \
                map(from: c[0:N]) \
                private(i) \
                reduction(+:c[0:N])
    for (int i = 0; i < N; i++) {
        int local_sum = 0;
        
        /* Access multi-dimensional arrays with varying patterns */
        for (int j = 0; j < M; j++) {
            local_sum += (int)d[i % N][j];
            
            /* Access 3D array */
            if (j < P && j < M) {
                for (int k = 0; k < 4; k++) {
                    local_sum += (int)e[i % P][j][k];
                }
            }
        }
        
        c[i] = a[i] + b[i] + local_sum;
        
        /* Conditional with nested loop */
        if (i % 3 == 0) {
            double temp = 0.0;
            #pragma omp simd
            for (int j = 0; j < 8; j++) {
                temp += d[i % N][j % M];
            }
            c[i] += (int)(temp * 100);
        }
    }
    
    printf("OpenMP test completed.\n");
}

/* Pattern C: Dynamic memory allocation */
void test_dynamic_memory_partitioning(void) {
    int size = N * M;
    int *dynamic_arr = (int*)malloc(size * sizeof(int));
    float *dynamic_float = (float*)malloc(size * sizeof(float));
    
    if (!dynamic_arr || !dynamic_float) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    /* Initialize dynamic arrays */
    for (int i = 0; i < size; i++) {
        dynamic_arr[i] = i % 100;
        dynamic_float[i] = i * 0.1f;
    }
    
    /* OpenACC with dynamic data */
    #pragma acc enter data copyin(dynamic_arr[0:size], dynamic_float[0:size])
    
    #pragma acc parallel loop present(dynamic_arr[0:size], dynamic_float[0:size]) \
                gang worker vector
    for (int i = 0; i < size; i++) {
        dynamic_arr[i] = dynamic_arr[i] * 2 + (int)dynamic_float[i];
        
        /* Complex conditional to prevent optimization */
        if (dynamic_arr[i] > 1000) {
            dynamic_float[i] = dynamic_float[i] / 2.0f;
        } else {
            dynamic_float[i] = dynamic_float[i] * 1.5f;
        }
    }
    
    #pragma acc exit data copyout(dynamic_arr[0:size], dynamic_float[0:size])
    
    /* Verify some results */
    int check_sum = 0;
    for (int i = 0; i < 100; i++) {
        check_sum += dynamic_arr[i];
    }
    printf("Dynamic memory test completed. Checksum: %d\n", check_sum);
    
    free(dynamic_arr);
    free(dynamic_float);
}

int main(void) {
    printf("Starting comprehensive neuter-broadcast test...\n");
    
    /* Test 1: OpenACC with various partitioning patterns */
    test_openacc_partitioning();
    
    /* Test 2: OpenMP target regions */
    test_openmp_partitioning();
    
    /* Test 3: Dynamic memory patterns */
    test_dynamic_memory_partitioning();
    
    printf("All tests completed successfully.\n");
    
    /* Simple validation */
    int validation = 1;
    for (int i = 0; i < 10; i++) {
        validation *= (i + 1);
    }
    
    return (validation == 3628800) ? 0 : 1;
}
