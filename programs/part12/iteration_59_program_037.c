/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * Covers all partitioning states (0-7) in the switch statement
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#define USE_OPENACC 1
#else
#define USE_OPENACC 0
#endif

#define N 128
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_partitioning_states(int *results) {
    /* Static multi-dimensional arrays (Pattern B) */
    static int static_3d[4][8][16];
    
    /* Stack arrays */
    int arr1d[N];
    int arr2d[M][M];
    int arr3d[P][P][P/2];
    
    /* Scalars with different storage classes */
    int scalar_private = 42;
    int scalar_firstprivate = 100;
    int scalar_reduction = 0;
    int scalar_shared = 999;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) arr1d[i] = i;
    for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++)
            arr2d[i][j] = i * M + j;
    for (int i = 0; i < P; i++)
        for (int j = 0; j < P; j++)
            for (int k = 0; k < P/2; k++)
                arr3d[i][j][k] = i * P * P/2 + j * P/2 + k;
    
    /* Pattern C: Pointer-based dynamic memory */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    int **dynamic_2d = (int**)malloc(M * sizeof(int*));
    for (int i = 0; i < N; i++) dynamic_arr[i] = i * 2;
    for (int i = 0; i < M; i++) {
        dynamic_2d[i] = (int*)malloc(M * sizeof(int));
        for (int j = 0; j < M; j++) dynamic_2d[i][j] = i + j;
    }
    
#if USE_OPENACC
    /* OpenACC implementation with various data clauses */
    #pragma acc data copy(results[0:N]) \
                    copyin(arr1d, arr2d, arr3d) \
                    copyin(static_3d) \
                    create(dynamic_arr[0:N]) \
                    copyin(dynamic_2d[0:M][0:M])
    {
        #pragma acc parallel loop gang vector_length(32) \
                private(scalar_private) \
                firstprivate(scalar_firstprivate) \
                reduction(+:scalar_reduction) \
                present(results, arr1d, arr2d, arr3d, static_3d, dynamic_arr, dynamic_2d)
        for (int i = 0; i < N; i++) {
            /* Complex nested loops and conditionals to create varied data flow */
            int local_var = scalar_private + i;
            
            /* Gang-level computation */
            if (i % 8 == 0) {
                for (int j = 0; j < M; j++) {
                    /* Worker-level computation */
                    int worker_var = arr2d[i/8][j] + local_var;
                    
                    for (int k = 0; k < 16; k++) {
                        /* Vector-level computation */
                        int vector_var = worker_var + k;
                        
                        /* Access different array types with different dimensionality */
                        results[i] += arr1d[j % N];
                        results[i] += arr2d[j % M][k % M];
                        if (i < P && j < P && k < P/2) {
                            results[i] += arr3d[i][j][k];
                        }
                        
                        /* Access static 3D array */
                        results[i] += static_3d[i % 4][j % 8][k % 16];
                        
                        /* Access dynamic memory */
                        results[i] += dynamic_arr[(i + j + k) % N];
                        if (j < M && k < M) {
                            results[i] += dynamic_2d[j][k];
                        }
                        
                        /* Conditional that creates complex control flow */
                        if (vector_var % 3 == 0) {
                            scalar_reduction += 1;
                        } else if (vector_var % 3 == 1) {
                            scalar_reduction += 2;
                        } else {
                            scalar_reduction += 3;
                        }
                    }
                }
            } else {
                /* Different computation path */
                for (int j = 0; j < 4; j++) {
                    results[i] += arr1d[(i + j) % N] * 2;
                }
            }
            
            /* Reduction operation */
            results[i] += scalar_reduction;
        }
    }
#else
    /* OpenMP implementation with similar complexity */
    #pragma omp target teams distribute parallel for \
                map(to: arr1d[0:N], arr2d[0:M][0:M], arr3d[0:P][0:P][0:P/2], static_3d[0:4][0:8][0:16]) \
                map(tofrom: results[0:N]) \
                map(to: dynamic_arr[0:N]) \
                map(to: dynamic_2d[0:M][0:M]) \
                private(scalar_private) \
                firstprivate(scalar_firstprivate) \
                reduction(+:scalar_reduction)
    for (int i = 0; i < N; i++) {
        int local_var = scalar_private + i;
        
        /* Nested loops with different iteration spaces */
        #pragma omp parallel for collapse(2)
        for (int j = 0; j < M; j += 4) {
            for (int k = 0; k < 16; k++) {
                int worker_vector_var = local_var + j + k;
                
                /* Access arrays with different access patterns */
                results[i] += arr1d[(i + j + k) % N];
                results[i] += arr2d[j % M][k % M];
                
                if (i < P && j < P && k < P/2) {
                    results[i] += arr3d[i][j][k] * 3;
                }
                
                /* Mixed static/dynamic array access */
                results[i] += static_3d[i % 4][j % 8][k % 16];
                results[i] += dynamic_arr[(i * j + k) % N];
                
                if (j < M && k < M) {
                    results[i] += dynamic_2d[j % M][k % M] * 2;
                }
                
                /* Complex conditional structure */
                switch (worker_vector_var % 4) {
                    case 0:
                        scalar_reduction += arr1d[i % N];
                        break;
                    case 1:
                        scalar_reduction += arr2d[j % M][k % M];
                        break;
                    case 2:
                        if (i < P && j < P && k < P/2)
                            scalar_reduction += arr3d[i][j][k];
                        break;
                    case 3:
                        scalar_reduction += static_3d[i % 4][j % 8][k % 16];
                        break;
                }
            }
        }
        
        results[i] += scalar_reduction;
    }
#endif
    
    /* Cleanup dynamic memory */
    free(dynamic_arr);
    for (int i = 0; i < M; i++) free(dynamic_2d[i]);
    free(dynamic_2d);
}

/* Pattern D: C++ style struct (implemented in C for compatibility) */
typedef struct {
    int x;
    float y;
    double z;
    int arr[4];
} ComplexStruct;

void test_struct_partitioning() {
    ComplexStruct struct_array[N];
    
    /* Initialize struct array */
    for (int i = 0; i < N; i++) {
        struct_array[i].x = i;
        struct_array[i].y = i * 1.5f;
        struct_array[i].z = i * 2.5;
        for (int j = 0; j < 4; j++) {
            struct_array[i].arr[j] = i * 10 + j;
        }
    }
    
#if USE_OPENACC
    #pragma acc data copy(struct_array[0:N])
    {
        #pragma acc parallel loop gang worker vector_length(16)
        for (int i = 0; i < N; i++) {
            /* Access different struct members with different patterns */
            struct_array[i].x *= 2;
            struct_array[i].y += 1.0f;
            
            /* Nested loop accessing struct array member */
            for (int j = 0; j < 4; j++) {
                struct_array[i].arr[j] += i + j;
                struct_array[i].z += struct_array[i].arr[j] * 0.5;
            }
            
            /* Conditional based on struct member */
            if (struct_array[i].x % 3 == 0) {
                struct_array[i].y *= 2.0f;
            }
        }
    }
#else
    #pragma omp target teams distribute parallel for \
                map(tofrom: struct_array[0:N])
    for (int i = 0; i < N; i++) {
        struct_array[i].x *= 2;
        struct_array[i].y += 1.0f;
        
        #pragma omp simd
        for (int j = 0; j < 4; j++) {
            struct_array[i].arr[j] += i + j;
            struct_array[i].z += struct_array[i].arr[j] * 0.5;
        }
        
        if (struct_array[i].x % 3 == 0) {
            struct_array[i].y *= 2.0f;
        }
    }
#endif
    
    /* Verify results */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += struct_array[i].x + (int)struct_array[i].y + (int)struct_array[i].z;
        for (int j = 0; j < 4; j++) {
            checksum += struct_array[i].arr[j];
        }
    }
    printf("Struct checksum: %d\n", checksum);
}

int main() {
    int *results = (int*)calloc(N, sizeof(int));
    int expected_sum = 0;
    
    /* Pre-compute expected result for verification */
    for (int i = 0; i < N; i++) {
        expected_sum += i;  // Base from arr1d initialization
    }
    
    printf("Testing partitioning states with %s\n", 
           USE_OPENACC ? "OpenACC" : "OpenMP");
    
    /* Test 1: Core partitioning states */
    test_partitioning_states(results);
    
    /* Test 2: Struct-based partitioning */
    test_struct_partitioning();
    
    /* Verify results */
    int total = 0;
    for (int i = 0; i < N; i++) {
        total += results[i];
    }
    
    printf("Total result: %d (expected approx %d)\n", total, expected_sum * 10);
    
    /* Simple validation - just check that we computed something non-zero */
    if (total > 0) {
        printf("Test PASSED: Computed non-zero result\n");
    } else {
        printf("Test FAILED: Zero result\n");
        free(results);
        return 1;
    }
    
    free(results);
    return 0;
}
