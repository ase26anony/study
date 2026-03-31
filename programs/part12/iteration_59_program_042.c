/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in GCC's
 * omp-oacc-neuter-broadcast.cc switch statement (cases 0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#define TARGET_PRAGMA acc parallel loop
#define DATA_COPYIN copyin
#define DATA_COPYOUT copyout
#define DATA_CREATE create
#define DATA_PRIVATE private
#define DATA_FIRSTPRIVATE firstprivate
#define DATA_REDUCTION reduction
#else
#define TARGET_PRAGMA omp target teams distribute parallel for
#define DATA_COPYIN map(to)
#define DATA_COPYOUT map(from)
#define DATA_CREATE map(alloc)
#define DATA_PRIVATE private
#define DATA_FIRSTPRIVATE firstprivate
#define DATA_REDUCTION reduction
#endif

#define N 1024
#define M 64
#define P 8

/* Struct to test complex data partitioning */
typedef struct {
    int x;
    float y;
    double z[4];
} DataStruct;

/* Function containing complex parallel region */
void test_partitioning_states(int n, int m, int p) {
    /* Pattern A: Various scalar and array types with different clauses */
    int scalar_private;                     /* Likely gang redundant (0) */
    int scalar_firstprivate = 42;           /* Likely gang redundant (0) */
    int reduction_sum = 0;                  /* Reduction variable */
    
    /* 1D arrays with different mappings */
    int arr1d[N];                           /* Base array */
    float arr1d_float[N];                   /* Different type */
    
    /* Pattern B: Multi-dimensional arrays */
    int arr3d[M][M][M];                     /* 3D array for complex partitioning */
    double matrix[N][N];                    /* 2D array */
    
    /* Pattern C: Pointer-based dynamic data */
    int *dynamic_arr;
    float **jagged_arr;
    
    /* Pattern D: Struct-based data */
    DataStruct struct_arr[N];
    
    /* Initialize host data */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i;
        arr1d_float[i] = i * 1.5f;
        struct_arr[i].x = i;
        struct_arr[i].y = i * 2.0f;
        for (int k = 0; k < 4; k++) {
            struct_arr[i].z[k] = i * 3.0 + k;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < M; k++) {
                arr3d[i][j][k] = i * M * M + j * M + k;
            }
        }
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = i * N + j;
        }
    }
    
    /* Allocate and initialize dynamic arrays */
    dynamic_arr = (int*)malloc(N * sizeof(int));
    jagged_arr = (float**)malloc(M * sizeof(float*));
    
    for (int i = 0; i < N; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    for (int i = 0; i < M; i++) {
        jagged_arr[i] = (float*)malloc(P * sizeof(float));
        for (int j = 0; j < P; j++) {
            jagged_arr[i][j] = i * P + j;
        }
    }
    
    /* Complex parallel region with multiple data clauses */
    #pragma TARGET_PRAGMA \
        DATA_PRIVATE(scalar_private) \
        DATA_FIRSTPRIVATE(scalar_firstprivate) \
        DATA_REDUCTION(+:reduction_sum) \
        DATA_COPYIN(arr1d[0:N], arr1d_float[0:N]) \
        DATA_COPYOUT(matrix[0:N][0:N]) \
        DATA_CREATE(arr3d[0:M][0:M][0:M]) \
        DATA_COPYIN(struct_arr[0:N])
    for (int i = 0; i < n; i++) {
        /* Access private scalar */
        scalar_private = i;
        
        /* Pattern A: Simple array access - may trigger gang partitioned (1) */
        int temp = arr1d[i];
        
        /* Pattern B: Multi-dimensional array access with complex indexing
         * This may trigger various partitioning states based on which
         * dimensions are parallelized */
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                /* 3D array access - may trigger gang+worker partitioned (3)
                 * or gang+vector partitioned (5) depending on loop nesting */
                if (i < M && j < M && k < M) {
                    arr3d[i][j][k] += temp;
                }
                
                /* 2D matrix access - may trigger worker partitioned (2) 
                 * or vector partitioned (4) */
                if (i < N && j < N) {
                    matrix[i][j] += arr1d_float[i] * j;
                }
            }
        }
        
        /* Pattern D: Struct member access - may trigger different partitioning
         * for different members */
        struct_arr[i].x = struct_arr[i].x * 2;
        struct_arr[i].y = struct_arr[i].y * 1.5f;
        for (int k = 0; k < 4; k++) {
            struct_arr[i].z[k] += k;
        }
        
        /* Reduction operation */
        reduction_sum += temp;
        
        /* Conditional with nested loops to create complex data flow */
        if (i % 2 == 0) {
            /* Additional nested computation */
            float acc = 0.0f;
            for (int j = 0; j < 10; j++) {
                for (int k = 0; k < 10; k++) {
                    acc += arr1d_float[(i + j + k) % N];
                }
            }
            arr1d_float[i] = acc;
        }
    }
    
    /* Second parallel region with different data mapping */
    int arr2[N];
    #pragma TARGET_PRAGMA \
        DATA_COPYIN(arr1d[0:N]) \
        DATA_COPYOUT(arr2[0:N])
    for (int i = 0; i < n; i++) {
        /* Different access pattern to trigger different partitioning */
        arr2[i] = arr1d[i] * 2;
        
        /* Nested loop with vector-like operations */
        for (int j = 0; j < 16; j++) {
            arr2[i] += j;
        }
    }
    
    /* Third region with pointer data */
    #ifdef _OPENACC
    #pragma acc enter data copyin(dynamic_arr[0:N])
    #pragma acc enter data copyin(jagged_arr[0:M])
    for (int i = 0; i < M; i++) {
        #pragma acc enter data copyin(jagged_arr[i][0:P])
    }
    #else
    #pragma omp target map(to: dynamic_arr[0:N])
    #pragma omp target map(to: jagged_arr[0:M])
    for (int i = 0; i < M; i++) {
        #pragma omp target map(to: jagged_arr[i][0:P])
        {}
    }
    #endif
    
    int ptr_sum = 0;
    #pragma TARGET_PRAGMA \
        DATA_REDUCTION(+:ptr_sum)
    for (int i = 0; i < n; i++) {
        /* Pattern C: Pointer access - may trigger worker+vector partitioned (6)
         * or fully partitioned (7) */
        ptr_sum += dynamic_arr[i];
        
        /* Jagged array access */
        if (i < M) {
            for (int j = 0; j < P; j++) {
                ptr_sum += (int)jagged_arr[i][j];
            }
        }
    }
    
    /* Cleanup */
    #ifdef _OPENACC
    #pragma acc exit data delete(dynamic_arr)
    #pragma acc exit data delete(jagged_arr)
    for (int i = 0; i < M; i++) {
        #pragma acc exit data delete(jagged_arr[i])
    }
    #endif
    
    for (int i = 0; i < M; i++) {
        free(jagged_arr[i]);
    }
    free(jagged_arr);
    free(dynamic_arr);
    
    /* Use results to prevent optimization */
    printf("Reduction sum: %d, Pointer sum: %d\n", reduction_sum, ptr_sum);
    printf("Sample values: arr1d[10]=%d, matrix[5][5]=%.2f\n", 
           arr1d[10], matrix[5][5]);
}

/* Main function with validation */
int main() {
    int n = N;
    int m = M;
    int p = P;
    
    printf("Testing partitioning states with n=%d, m=%d, p=%d\n", n, m, p);
    
    /* Call function multiple times with different parameters
     * to increase chance of hitting all partitioning states */
    test_partitioning_states(n, m, p);
    test_partitioning_states(n/2, m/2, p/2);
    test_partitioning_states(n*2, m, p);  /* Different shape */
    
    /* Additional test with different data sizes */
    {
        int small_arr[16];
        int medium_arr[256];
        int large_arr[4096];
        
        #pragma TARGET_PRAGMA DATA_COPYIN(small_arr, medium_arr, large_arr)
        for (int i = 0; i < 16; i++) {
            small_arr[i] = i;
            if (i < 256) medium_arr[i] = i * 2;
            if (i < 4096) large_arr[i] = i * 3;
        }
        
        int check = 0;
        #pragma TARGET_PRAGMA \
            DATA_COPYIN(small_arr, medium_arr, large_arr) \
            DATA_REDUCTION(+:check)
        for (int i = 0; i < 4096; i++) {
            if (i < 16) check += small_arr[i];
            if (i < 256) check += medium_arr[i];
            if (i < 4096) check += large_arr[i];
        }
        
        printf("Check sum: %d\n", check);
    }
    
    printf("Test completed successfully.\n");
    return 0;
}
