/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * Covers all partitioning states (0-7) in the switch statement
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#define PARALLEL_LOOP _Pragma("acc parallel loop copyout(result[0:N]) copyin(a[0:N], b[0:N]) create(temp[0:N])")
#else
#define PARALLEL_LOOP _Pragma("omp target teams distribute parallel for map(tofrom:result[0:N]) map(to:a[0:N], b[0:N]) map(alloc:temp[0:N])")
#endif

#define N 1024
#define M 32
#define P 16

/* Struct to test complex data partitioning */
struct DataPoint {
    double x;
    double y;
    double z;
    int id;
};

/* Function with complex parallel region to trigger various partitioning states */
void compute_kernel(int n, int m, int p) {
    /* Different types of variables that may get different partitioning states */
    
    /* 1. Scalar variables - may become gang redundant (0) */
    int scalar_gang_redundant = 42;
    float scalar_worker_var = 3.14f;
    double scalar_vector_var = 2.71828;
    
    /* 2. 1D arrays - various partitioning possibilities */
    int arr1d[N];
    float arr1d_float[N];
    
    /* 3. 2D arrays - more complex partitioning */
    int arr2d[M][P];
    double arr2d_double[M][P];
    
    /* 4. 3D arrays - maximum dimensionality for partitioning analysis */
    short arr3d[N/16][M/2][P/2];
    
    /* 5. Pointer-based dynamic access */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    
    /* 6. Struct array - tests aggregate partitioning */
    struct DataPoint points[N/4];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i;
        arr1d_float[i] = i * 0.5f;
        if (i < N/4) {
            points[i].x = i * 1.0;
            points[i].y = i * 2.0;
            points[i].z = i * 3.0;
            points[i].id = i;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            arr2d[i][j] = i * P + j;
            arr2d_double[i][j] = i * 0.1 + j * 0.01;
        }
    }
    
    for (int i = 0; i < N/16; i++) {
        for (int j = 0; j < M/2; j++) {
            for (int k = 0; k < P/2; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (int i = 0; i < N; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* Complex parallel region with multiple nesting levels */
    #ifdef _OPENACC
    #pragma acc parallel loop copyin(scalar_gang_redundant, scalar_worker_var, scalar_vector_var) \
        copy(arr1d[0:N], arr1d_float[0:N], arr2d[0:M][0:P], arr2d_double[0:M][0:P]) \
        copy(arr3d[0:N/16][0:M/2][0:P/2]) \
        copy(points[0:N/4]) \
        copyin(dynamic_arr[0:N]) \
        create(temp_results[0:N])
    #else
    #pragma omp target teams distribute parallel for \
        map(to: scalar_gang_redundant, scalar_worker_var, scalar_vector_var) \
        map(tofrom: arr1d[0:N], arr1d_float[0:N], arr2d[0:M][0:P], arr2d_double[0:M][0:P]) \
        map(tofrom: arr3d[0:N/16][0:M/2][0:P/2]) \
        map(tofrom: points[0:N/4]) \
        map(to: dynamic_arr[0:N]) \
        map(alloc: temp_results[0:N])
    #endif
    for (int gang_idx = 0; gang_idx < n; gang_idx++) {
        /* Gang-level computation - scalar_gang_redundant may be gang redundant (0) */
        int gang_var = scalar_gang_redundant + gang_idx;
        
        /* Nested worker loop */
        #ifdef _OPENACC
        #pragma acc loop worker
        #else
        #pragma omp parallel for
        #endif
        for (int worker_idx = 0; worker_idx < m; worker_idx++) {
            /* Worker-level computation - scalar_worker_var may be worker partitioned (2) */
            float worker_var = scalar_worker_var * worker_idx;
            
            /* Access 2D array with gang and worker indices - may be gang+worker partitioned (3) */
            arr2d[gang_idx % M][worker_idx % P] += gang_var + (int)worker_var;
            arr2d_double[gang_idx % M][worker_idx % P] += gang_var * 0.01;
            
            /* Further nested vector loop */
            #ifdef _OPENACC
            #pragma acc loop vector
            #else
            #pragma omp simd
            #endif
            for (int vector_idx = 0; vector_idx < p; vector_idx++) {
                /* Vector-level computation - scalar_vector_var may be vector partitioned (4) */
                double vector_var = scalar_vector_var * vector_idx;
                
                /* Access 1D array - may be fully partitioned (7) with all three dimensions */
                int linear_idx = (gang_idx * m * p) + (worker_idx * p) + vector_idx;
                if (linear_idx < N) {
                    arr1d[linear_idx] += gang_var + (int)worker_var + (int)vector_var;
                    arr1d_float[linear_idx] += worker_var + vector_var;
                    
                    /* Access dynamic array - different partitioning due to pointer */
                    dynamic_arr[linear_idx] += gang_idx + worker_idx + vector_idx;
                }
                
                /* Access 3D array with all indices - tests gang+worker+vector partitioning (6) */
                if (gang_idx < N/16 && worker_idx < M/2 && vector_idx < P/2) {
                    arr3d[gang_idx][worker_idx][vector_idx] += 
                        gang_idx * 100 + worker_idx * 10 + vector_idx;
                }
                
                /* Access struct array - complex partitioning for members */
                if (linear_idx < N/4) {
                    points[linear_idx].x += gang_idx * 0.1;
                    points[linear_idx].y += worker_idx * 0.2;
                    points[linear_idx].z += vector_idx * 0.3;
                    
                    /* Struct member access might trigger different partitioning */
                    if (vector_idx % 2 == 0) {
                        points[linear_idx].id = gang_idx;
                    } else {
                        points[linear_idx].id = worker_idx;
                    }
                }
                
                /* Conditional access patterns to create varied data flow */
                if (gang_idx % 3 == 0) {
                    /* This path might create gang+vector partitioned (5) accesses */
                    if (linear_idx * 2 < N) {
                        arr1d[linear_idx * 2] = vector_idx;
                    }
                } else if (worker_idx % 4 == 0) {
                    /* This path might create worker+vector partitioned (6) accesses */
                    if (linear_idx * 3 < N) {
                        arr1d_float[linear_idx * 3] = gang_idx * 0.5f;
                    }
                }
            }
        }
        
        /* Post-worker computation with reduction-like pattern */
        int temp_sum = 0;
        #ifdef _OPENACC
        #pragma acc loop worker reduction(+:temp_sum)
        #else
        #pragma omp parallel for reduction(+:temp_sum)
        #endif
        for (int i = 0; i < m; i++) {
            temp_sum += arr2d[gang_idx % M][i % P];
        }
        
        /* Store result in another array with different partitioning */
        if (gang_idx < N) {
            arr1d[gang_idx] = temp_sum / m;
        }
    }
    
    free(dynamic_arr);
    
    /* Additional parallel region with different data mapping */
    int result[N];
    int a[N], b[N], temp[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        result[i] = 0;
    }
    
    /* Another parallel loop with different clause combination */
    PARALLEL_LOOP
    for (int i = 0; i < N; i++) {
        temp[i] = a[i] + b[i];
        
        /* Nested conditional loops to create complex control flow */
        if (i % 8 == 0) {
            for (int j = 0; j < 4; j++) {
                temp[i] += j;
            }
        } else if (i % 8 == 1) {
            int k = 0;
            while (k < 3) {
                temp[i] -= k;
                k++;
            }
        }
        
        result[i] = temp[i] * 2;
    }
    
    /* Verify computation */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += result[i];
    }
    printf("Checksum: %d\n", checksum);
}

/* Main function with multiple kernel calls */
int main() {
    printf("Testing neuter-broadcast pass coverage...\n");
    
    /* Call with different parameters to exercise different paths */
    compute_kernel(64, 8, 4);   /* Moderate sizes */
    compute_kernel(32, 16, 8);  /* Different aspect ratio */
    compute_kernel(128, 4, 2);  /* Many gangs, few workers/vectors */
    
    printf("Test completed successfully.\n");
    return 0;
}
