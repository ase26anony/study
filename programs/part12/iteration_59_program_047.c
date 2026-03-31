/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * 
 * This program creates variables with diverse partitioning attributes to trigger
 * all cases in the partitioning state switch statement (cases 0-7).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Different types of variables that may get different partitioning states */
    int scalar_private;                /* Likely gang redundant (0) */
    int scalar_firstprivate = 42;      /* May be gang partitioned (1) */
    int reduction_sum = 0;             /* Reduction variable */
    
    /* Single-dimensional arrays */
    int arr1d[N];                      /* Worker partitioned (2) or vector partitioned (4) */
    int arr1d_copy[N];                 /* For verification */
    
    /* Pattern B: Multi-dimensional arrays */
    int arr3d[N][M][P];                /* Complex partitioning - may trigger gang+worker (3) */
    int arr2d[M][P];                   /* May trigger gang+vector (5) or worker+vector (6) */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i % 17;
        arr1d_copy[i] = arr1d[i];
    }
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                arr3d[i][j][k] = (i + j + k) % 23;
            }
        }
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            arr2d[i][j] = (i * j) % 31;
        }
    }
    
    /* OpenACC parallel region with complex data clauses and nested loops */
    #pragma acc parallel loop gang vector_length(32) \
        copy(arr1d[0:N]) copyin(arr1d_copy[0:N]) \
        copy(arr3d[0:N][0:M][0:P]) copy(arr2d[0:M][0:P]) \
        private(scalar_private) firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum)
    for (i = 0; i < N; i++) {
        scalar_private = i;
        
        /* Nested loops to create complex data flow */
        for (j = 0; j < M; j++) {
            /* Conditional operations */
            if (j % 2 == 0) {
                /* Access multi-dimensional arrays with different indices */
                for (k = 0; k < P; k++) {
                    /* Different partitioning patterns based on access */
                    arr3d[i][j][k] += arr1d[i] + scalar_firstprivate;
                    
                    /* Vector-level partitioning */
                    if (k < P/2) {
                        arr2d[j][k] = arr3d[i][j][k] % 11;
                    } else {
                        arr2d[j][k] = arr3d[i][j][k] % 13;
                    }
                }
            } else {
                /* Worker-level operations */
                int worker_local = j * 3;
                for (k = 0; k < P; k += 4) {
                    arr3d[i][j][k] += worker_local;
                }
            }
        }
        
        /* Reduction operation */
        reduction_sum += arr1d[i];
        
        /* Update 1D array with conditional */
        if (i % 3 == 0) {
            arr1d[i] = arr1d_copy[i] + scalar_private;
        } else if (i % 3 == 1) {
            arr1d[i] = arr1d_copy[i] * 2;
        } else {
            arr1d[i] = arr1d_copy[i] / 2;
        }
    }
    
    /* Verify results */
    int verify_sum = 0;
    for (i = 0; i < N; i++) {
        verify_sum += arr1d_copy[i];
    }
    
    printf("OpenACC: Reduction sum = %d, Expected = %d\n", reduction_sum, verify_sum);
}

/* Pattern C: Variable-length data and pointers */
void test_openacc_pointers() {
    int i, j;
    int size = 256;
    
    /* Dynamic memory allocation */
    int *dyn_arr1 = (int *)malloc(size * sizeof(int));
    int *dyn_arr2 = (int *)malloc(size * sizeof(int));
    int **ptr_array = (int **)malloc(16 * sizeof(int *));
    
    /* Initialize dynamic arrays */
    for (i = 0; i < size; i++) {
        dyn_arr1[i] = i % 19;
        dyn_arr2[i] = (i * 2) % 23;
    }
    
    for (i = 0; i < 16; i++) {
        ptr_array[i] = (int *)malloc(32 * sizeof(int));
        for (j = 0; j < 32; j++) {
            ptr_array[i][j] = (i + j) % 29;
        }
    }
    
    /* OpenACC with pointer-based data */
    #pragma acc enter data copyin(dyn_arr1[0:size], dyn_arr2[0:size])
    
    #pragma acc parallel loop gang worker vector_length(64) \
        present(dyn_arr1[0:size], dyn_arr2[0:size]) \
        copy(ptr_array[0:16][0:32])
    for (i = 0; i < size; i++) {
        /* Complex pointer arithmetic */
        int idx = i % 16;
        
        /* Access through pointer array - may trigger fully partitioned (7) */
        for (j = 0; j < 32; j++) {
            ptr_array[idx][j] += dyn_arr1[i] + dyn_arr2[(i + j) % size];
        }
        
        /* Update dynamic arrays with conditional */
        if (dyn_arr1[i] > 10) {
            dyn_arr2[i] = dyn_arr1[i] * 3;
        } else {
            dyn_arr2[i] = dyn_arr1[i] + 100;
        }
    }
    
    #pragma acc exit data copyout(dyn_arr2[0:size])
    
    /* Cleanup */
    free(dyn_arr1);
    free(dyn_arr2);
    for (i = 0; i < 16; i++) {
        free(ptr_array[i]);
    }
    free(ptr_array);
}

/* Pattern D: C++ style struct (in C for compatibility) */
typedef struct {
    int x;
    float y;
    double z;
    int arr[8];
} ComplexData;

void test_openacc_structs() {
    int i, j;
    ComplexData data_array[N];
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        data_array[i].x = i;
        data_array[i].y = i * 1.5f;
        data_array[i].z = i * 2.5;
        for (j = 0; j < 8; j++) {
            data_array[i].arr[j] = (i + j) % 17;
        }
    }
    
    /* OpenACC with struct array - may trigger various partitioning states */
    #pragma acc parallel loop gang worker vector_length(16) \
        copy(data_array[0:N])
    for (i = 0; i < N; i++) {
        /* Access different struct members */
        data_array[i].x += (int)(data_array[i].y * 2.0f);
        data_array[i].z = data_array[i].x * 3.14;
        
        /* Nested loop within struct member array */
        for (j = 0; j < 8; j++) {
            if (j % 2 == 0) {
                data_array[i].arr[j] = data_array[i].x + j;
            } else {
                data_array[i].arr[j] = data_array[i].x - j;
            }
        }
        
        /* Conditional struct member update */
        if (data_array[i].x > 50) {
            data_array[i].y *= 1.1f;
        }
    }
}

/* OpenMP version to trigger different code paths */
void test_openmp_partitioning() {
    int i, j, k;
    int arr_omp[N][M];
    int scalar_omp = 100;
    int reduction_omp = 0;
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr_omp[i][j] = (i * j) % 37;
        }
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr_omp) map(to: scalar_omp) reduction(+:reduction_omp) \
        private(k) collapse(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            /* Complex nested operations */
            int local_var = i + j;
            
            for (k = 0; k < 4; k++) {
                arr_omp[i][j] += local_var * k;
            }
            
            /* Conditional with scalar */
            if (arr_omp[i][j] > scalar_omp) {
                arr_omp[i][j] = scalar_omp;
            } else {
                arr_omp[i][j] += scalar_omp / 2;
            }
            
            reduction_omp += arr_omp[i][j];
        }
    }
    
    printf("OpenMP: Processed %d x %d array\n", N, M);
}

/* Main function that runs all tests */
int main() {
    printf("Starting comprehensive neuter-broadcast test...\n");
    
    /* Test 1: OpenACC with mixed variables */
    printf("\n=== Test 1: OpenACC Mixed Variables ===\n");
    test_openacc_partitioning();
    
    /* Test 2: OpenACC with pointers */
    printf("\n=== Test 2: OpenACC Pointers ===\n");
    test_openacc_pointers();
    
    /* Test 3: OpenACC with structs */
    printf("\n=== Test 3: OpenACC Structs ===\n");
    test_openacc_structs();
    
    /* Test 4: OpenMP target regions */
    printf("\n=== Test 4: OpenMP Target ===\n");
    test_openmp_partitioning();
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
