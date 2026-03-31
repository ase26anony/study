/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * 
 * This program uses OpenACC constructs to create variables with different
 * partitioning states, aiming to trigger all cases in the switch statement
 * that maps partitioning codes to human-readable strings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 8

/* Simple struct to test aggregate partitioning */
struct DataPoint {
    double value;
    int index;
    float weight;
};

/* Function containing complex OpenACC region with diverse variable usage */
void process_data(int n, int m, int p) {
    /* Pattern A: Various scalar and array variables with different clauses */
    int scalar_private = 42;                     /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;               /* Likely gang redundant (0) */
    int reduction_sum = 0;                       /* Reduction variable */
    
    /* 1D arrays with different mappings */
    int arr1d[N];                                /* Base array */
    int arr1d_copy[N];                           /* For copy clause */
    int arr1d_create[N];                         /* For create clause */
    
    /* Pattern B: Multi-dimensional arrays */
    int arr3d[M][M][M];                          /* 3D array for complex partitioning */
    float matrix[M][M];                          /* 2D matrix */
    
    /* Pattern C: Pointer-based dynamic data */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    double *dynamic_matrix = (double*)malloc(M * M * sizeof(double));
    
    /* Pattern D: Array of structs */
    struct DataPoint data_points[N];
    
    /* Initialize data on host */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i % 100;
        arr1d_copy[i] = i % 50;
        arr1d_create[i] = 0;
        data_points[i].value = i * 0.1;
        data_points[i].index = i;
        data_points[i].weight = i * 0.01f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = i * j * 0.1f;
            for (int k = 0; k < M; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (int i = 0; i < N; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    for (int i = 0; i < M * M; i++) {
        dynamic_matrix[i] = i * 0.5;
    }
    
    /* Complex OpenACC parallel region with multiple clauses and nesting */
    #pragma acc parallel loop gang worker vector \
        copy(arr1d[0:N], arr1d_copy[0:N]) \
        copyin(matrix[0:M][0:M]) \
        copyout(arr1d_create[0:N]) \
        create(arr3d[0:M][0:M][0:M]) \
        present_or_copyin(dynamic_arr[0:N]) \
        present_or_copyin(dynamic_matrix[0:M*M]) \
        copy(data_points[0:N]) \
        private(scalar_private) \
        firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum) \
        async(1)
    for (int i = 0; i < n; i++) {
        /* Access scalar private variables */
        int local_var = scalar_private + i;
        
        /* Pattern A: Different array accesses */
        arr1d_create[i] = arr1d[i] + arr1d_copy[i];
        
        /* Pattern B: Multi-dimensional array access with complex indexing */
        int idx1 = i % M;
        int idx2 = (i / M) % M;
        int idx3 = (i / (M * M)) % M;
        
        if (idx1 < M && idx2 < M && idx3 < M) {
            /* This creates complex data flow for partitioning analysis */
            arr3d[idx1][idx2][idx3] += local_var;
            
            /* Nested loops inside parallel region */
            #pragma acc loop seq
            for (int j = 0; j < p; j++) {
                /* Conditional operations increase analysis complexity */
                if (j % 2 == 0) {
                    matrix[idx1][idx2] += arr3d[idx1][idx2][idx3] * 0.1f;
                } else {
                    matrix[idx2][idx1] -= arr3d[idx1][idx2][idx3] * 0.05f;
                }
            }
        }
        
        /* Pattern C: Pointer access with offset */
        if (i < N) {
            dynamic_arr[i] = dynamic_arr[i] * 2 + i;
        }
        
        /* Pattern D: Struct member access */
        data_points[i].value = data_points[i].value * 2.0 + 
                               data_points[i].weight * i;
        
        /* Reduction operation */
        reduction_sum += arr1d[i];
        
        /* Vector-level operation simulation */
        #pragma acc loop vector
        for (int v = 0; v < 4; v++) {
            /* This inner loop creates vector partitioning requirements */
            int vector_idx = (i * 4 + v) % N;
            if (vector_idx < N) {
                arr1d_copy[vector_idx] += v;
            }
        }
    }
    
    /* Wait for async operation */
    #pragma acc wait(1)
    
    /* Additional parallel region with different characteristics */
    int worker_partitioned_var = 0;
    int gang_partitioned_var = 0;
    
    #pragma acc parallel loop gang \
        copy(worker_partitioned_var, gang_partitioned_var) \
        present_or_copy(dynamic_arr[0:N])
    for (int g = 0; g < m; g++) {
        gang_partitioned_var += g;
        
        #pragma acc loop worker
        for (int w = 0; w < p; w++) {
            worker_partitioned_var += w;
            
            #pragma acc loop vector
            for (int v = 0; v < 4; v++) {
                int idx = (g * p * 4 + w * 4 + v) % N;
                if (idx < N) {
                    dynamic_arr[idx] += g + w + v;
                }
            }
        }
    }
    
    /* Clean up */
    free(dynamic_arr);
    free(dynamic_matrix);
    
    /* Use results to prevent optimization */
    printf("Reduction sum: %d\n", reduction_sum);
    printf("Worker partitioned var: %d\n", worker_partitioned_var);
    printf("Gang partitioned var: %d\n", gang_partitioned_var);
}

/* Second function with OpenMP target for additional coverage */
#ifdef _OPENMP
void process_data_omp(int n) {
    int arr[N];
    int arr2[N][M];
    int *dynamic = (int*)malloc(N * sizeof(int));
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        dynamic[i] = i * 3;
        for (int j = 0; j < M; j++) {
            arr2[i][j] = i + j;
        }
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr[0:N]) \
        map(to: arr2[0:N][0:M]) \
        map(tofrom: dynamic[0:N])
    for (int i = 0; i < n; i++) {
        int local = arr[i];
        
        /* Nested loops for complex partitioning */
        for (int j = 0; j < M; j++) {
            local += arr2[i][j];
            
            /* Conditional with vector-like operation */
            if (j % 4 == 0) {
                #pragma omp simd
                for (int k = 0; k < 8; k++) {
                    dynamic[i] += k;
                }
            }
        }
        
        arr[i] = local;
    }
    
    free(dynamic);
}
#endif

int main() {
    printf("Testing neuter-broadcast pass coverage...\n");
    
    /* Call OpenACC function */
    process_data(N, M, P);
    
    /* Call OpenMP function if available */
    #ifdef _OPENMP
    process_data_omp(N);
    printf("OpenMP version executed\n");
    #else
    printf("OpenMP not available, skipping OpenMP test\n");
    #endif
    
    /* Verify with a simple computation */
    int verify_arr[10];
    int sum = 0;
    
    #pragma acc parallel loop copyout(verify_arr[0:10]) reduction(+:sum)
    for (int i = 0; i < 10; i++) {
        verify_arr[i] = i * i;
        sum += verify_arr[i];
    }
    
    int expected_sum = 0;
    for (int i = 0; i < 10; i++) {
        expected_sum += i * i;
    }
    
    if (sum == expected_sum) {
        printf("Verification passed: sum = %d\n", sum);
        return 0;
    } else {
        printf("Verification failed: got %d, expected %d\n", sum, expected_sum);
        return 1;
    }
}
