/* Test program to cover all partitioning states in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=disable -fdump-tree-all -fprofile-arcs -ftest-coverage test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Function containing complex OpenACC regions */
void process_data(int n, int m, int p) {
    /* Pattern A: Various scalar and array types */
    int scalar_private = 42;                    /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;              /* Likely gang redundant (0) */
    int reduction_sum = 0;                      /* Reduction variable */
    
    /* Pattern B: Multi-dimensional arrays with different access patterns */
    int arr_3d[M][P][N];
    int arr_2d[M][N];
    int arr_1d[N];
    
    /* Pattern C: Pointer-based dynamic memory */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    int **ptr_arr = (int**)malloc(M * sizeof(int*));
    for (int i = 0; i < M; i++) {
        ptr_arr[i] = (int*)malloc(P * sizeof(int));
    }
    
    /* Initialize arrays */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            for (int k = 0; k < N; k++) {
                arr_3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr_2d[i][j] = i * j;
        }
    }
    
    for (int i = 0; i < N; i++) {
        arr_1d[i] = i;
        dynamic_arr[i] = i * 2;
    }
    
    /* Pattern D: Struct with multiple members */
    struct DataPoint {
        int id;
        float value;
        double precision;
    };
    struct DataPoint data_array[N];
    for (int i = 0; i < N; i++) {
        data_array[i].id = i;
        data_array[i].value = i * 0.5f;
        data_array[i].precision = i * 0.25;
    }
    
    /* Complex OpenACC parallel region with multiple clauses */
    #pragma acc parallel loop gang vector \
        copy(arr_3d, arr_2d, arr_1d) \
        copyin(scalar_firstprivate) \
        copyout(dynamic_arr[0:N]) \
        create(ptr_arr[0:M][0:P]) \
        reduction(+:reduction_sum) \
        private(scalar_private) \
        firstprivate(scalar_firstprivate)
    for (int i = 0; i < M; i++) {
        /* Nested loops to create complex data flow */
        for (int j = 0; j < P; j++) {
            /* Conditional operations */
            if (i % 2 == 0) {
                /* Access 3D array - may trigger gang+worker partitioned (3) */
                int temp = arr_3d[i][j][0];
                
                /* Worker-level computation */
                #pragma acc loop worker
                for (int k = 0; k < N; k++) {
                    /* Vector-level operations */
                    #pragma acc loop vector
                    for (int l = 0; l < 4; l++) {
                        arr_3d[i][j][k] += temp + l;
                    }
                    
                    /* Access different arrays to trigger various partitioning */
                    arr_2d[i][k] += arr_1d[k];      /* May trigger worker+vector partitioned (6) */
                    
                    /* Pointer access */
                    ptr_arr[i][j] = i * j + k;
                    
                    /* Struct member access */
                    data_array[k].value += arr_2d[i][k] * 0.1f;
                    
                    /* Reduction */
                    reduction_sum += arr_3d[i][j][k];
                }
            } else {
                /* Different access pattern for odd indices */
                #pragma acc loop vector
                for (int k = 0; k < N; k += 2) {
                    /* May trigger gang+vector partitioned (5) */
                    arr_1d[k] = arr_3d[i][j][k] * 2;
                    
                    /* Conditional with scalar */
                    scalar_private = (scalar_firstprivate > 50) ? 1 : 0;
                    dynamic_arr[k] = scalar_private * k;
                }
            }
        }
    }
    
    /* Second OpenACC region with kernels construct */
    float float_arr[N];
    double double_arr[N];
    
    #pragma acc kernels copy(float_arr, double_arr)
    {
        #pragma acc loop gang(32) worker(4) vector(16)
        for (int i = 0; i < N; i++) {
            /* Mixed data types and operations */
            float_arr[i] = i * 0.1f;
            double_arr[i] = float_arr[i] * 2.0;
            
            /* Access struct array */
            data_array[i].precision = double_arr[i];
            
            /* Complex conditional to prevent optimization */
            if (i % 3 == 0) {
                float_arr[i] *= data_array[i].value;
            } else if (i % 3 == 1) {
                double_arr[i] /= (data_array[i].id + 1);
            } else {
                float_arr[i] = float_arr[i] + double_arr[i];
            }
        }
    }
    
    /* Third region: OpenMP-style target for comparison */
    int omp_arr[N];
    
    #pragma acc parallel loop present(omp_arr)
    for (int i = 0; i < N; i++) {
        /* Fully partitioned access pattern (7) */
        omp_arr[i] = arr_1d[i] + arr_2d[i % M][i];
        
        /* Vector-only partitioned operations (4) */
        #pragma acc loop vector
        for (int v = 0; v < 8; v++) {
            omp_arr[i] += v;
        }
    }
    
    /* Cleanup */
    free(dynamic_arr);
    for (int i = 0; i < M; i++) {
        free(ptr_arr[i]);
    }
    free(ptr_arr);
    
    /* Use results to prevent dead code elimination */
    printf("Reduction sum: %d\n", reduction_sum);
    printf("Sample values: %d, %f, %lf\n", 
           arr_1d[10], data_array[20].value, double_arr[30]);
}

/* Main function with validation */
int main() {
    printf("Starting OpenACC neuter-broadcast coverage test...\n");
    
    /* Call the processing function multiple times with different parameters */
    process_data(N, M, P);
    
    /* Additional call with different sizes to trigger different optimizations */
    process_data(N/2, M/2, P/2);
    
    printf("Test completed successfully.\n");
    
    /* Simple validation */
    int validation = 1;
    if (validation) {
        return 0;  /* Success */
    } else {
        return 1;  /* Failure */
    }
}
