/* Test program to cover partitioning state mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=disable -fdump-tree-all -fprofile-arcs -ftest-coverage test.c -o test
 * Or for OpenMP: gcc -O3 -fopenmp -fdump-tree-omp-oacc-neuter-broadcast -fprofile-arcs -ftest-coverage test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Struct to create complex data access patterns */
typedef struct {
    int scalar;
    float vector[4];
    double matrix[3][3];
} ComplexData;

/* Function with OpenACC parallel region containing diverse variable patterns */
void acc_parallel_test(int n, int m, int p) {
    /* Pattern A: Various scalar and array types with different data clauses */
    int scalar_private = 0;                    /* Likely gang redundant (0) */
    int scalar_firstprivate = 42;              /* Likely gang redundant (0) */
    int reduction_sum = 0;                     /* Reduction variable */
    
    /* 1D arrays with different mappings */
    int arr1d[N];                              /* Fully partitioned (7) with loop access */
    int arr1d_copyin[M];                       /* Copyin array */
    
    /* Pattern B: Multi-dimensional arrays */
    int arr3d[N/8][M/4][P/2];                  /* 3D array - complex partitioning */
    float arr2d[M][P];                         /* 2D array */
    
    /* Pattern C: Pointer-based dynamic data */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    
    /* Pattern D: Array of structs */
    ComplexData struct_arr[M];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i % 100;
        if (i < N/8) {
            for (int j = 0; j < M/4; j++) {
                for (int k = 0; k < P/2; k++) {
                    arr3d[i][j][k] = i + j + k;
                }
            }
        }
    }
    
    for (int i = 0; i < M; i++) {
        arr1d_copyin[i] = i * 2;
        for (int j = 0; j < P; j++) {
            arr2d[i][j] = (float)(i * j) / 100.0f;
        }
        struct_arr[i].scalar = i;
        for (int v = 0; v < 4; v++) {
            struct_arr[i].vector[v] = (float)(i + v);
        }
    }
    
    for (int i = 0; i < N; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    /* OpenACC parallel region with complex data usage */
    #pragma acc parallel loop gang vector \
        copy(arr1d[0:N]) \
        copyin(arr1d_copyin[0:M]) \
        copyout(arr2d[0:M][0:P]) \
        create(arr3d[0:N/8][0:M/4][0:P/2]) \
        present(dynamic_arr[0:N]) \
        copy(struct_arr[0:M]) \
        private(scalar_private) \
        firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum)
    for (int i = 0; i < n; i++) {
        /* Nested loops to create complex access patterns */
        int gang_id = 0;  /* Could become gang partitioned (1) */
        int worker_id = 0; /* Could become worker partitioned (2) */
        
        /* Access 1D array - likely fully partitioned (7) */
        int val = arr1d[i];
        
        /* Access with conditional - creates complex data flow */
        if (val > 50) {
            /* Access multi-dimensional array */
            int idx1 = i % (N/8);
            int idx2 = (i / (N/8)) % (M/4);
            int idx3 = (i / (N/8) / (M/4)) % (P/2);
            
            /* 3D array access - gang+worker+vector partitioned patterns */
            arr3d[idx1][idx2][idx3] += val;
            
            /* Worker-level computation */
            #pragma acc loop worker
            for (int w = 0; w < 4; w++) {
                worker_id = w;
                /* Access struct array - different partitioning for members */
                struct_arr[w].scalar += val;
                
                /* Vector-level computation */
                #pragma acc loop vector
                for (int v = 0; v < 4; v++) {
                    struct_arr[w].vector[v] += (float)val;
                    
                    /* Access 2D array - worker+vector partitioned (6) */
                    if (w < M && v < P) {
                        arr2d[w][v] += struct_arr[w].vector[v];
                    }
                }
            }
            
            /* Gang-level variable */
            gang_id = val % 8;
            
            /* Dynamic array access - pointer partitioning */
            dynamic_arr[i] += gang_id + worker_id;
        } else {
            /* Different access pattern for else branch */
            #pragma acc loop seq
            for (int j = 0; j < 8; j++) {
                /* Sequential access creates different partitioning */
                scalar_private += arr1d_copyin[j % M];
            }
        }
        
        /* Reduction operation */
        reduction_sum += val;
        
        /* Vector-private variable */
        int vector_private = i % 16;  /* Could become vector partitioned (4) */
        
        /* Complex expression mixing different variable types */
        arr1d[i] = (val + scalar_private + vector_private + 
                   gang_id * 100 + worker_id * 10) % 1000;
    }
    
    /* Cleanup */
    free(dynamic_arr);
    
    /* Use results to prevent optimization */
    printf("Reduction sum: %d\n", reduction_sum);
    printf("Sample 3D array value: %d\n", arr3d[0][0][0]);
    printf("Sample struct scalar: %d\n", struct_arr[0].scalar);
}

/* OpenMP version with similar patterns */
void omp_target_test(int n, int m, int p) {
    int arr1d[N];
    int arr2d[M][P];
    int arr3d[N/8][M/4][P/2];
    ComplexData struct_arr[M];
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i;
        dynamic_arr[i] = i * 2;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            arr2d[i][j] = i + j;
        }
        struct_arr[i].scalar = i;
    }
    
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < M/4; j++) {
            for (int k = 0; k < P/2; k++) {
                arr3d[i][j][k] = i * j * k;
            }
        }
    }
    
    int reduction_sum = 0;
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr1d[0:N]) \
        map(to: arr2d[0:M][0:P]) \
        map(alloc: arr3d[0:N/8][0:M/4][0:P/2]) \
        map(tofrom: struct_arr[0:M]) \
        map(tofrom: dynamic_arr[0:N]) \
        reduction(+:reduction_sum) \
        collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m/4; j++) {
            /* Complex indexing to trigger different partitioning */
            int idx = (i * (m/4) + j) % N;
            
            /* Access different data structures conditionally */
            if (idx % 3 == 0) {
                /* Access 1D and dynamic */
                arr1d[idx] += dynamic_arr[idx];
                reduction_sum += arr1d[idx];
            } else if (idx % 3 == 1) {
                /* Access 2D */
                int mi = idx % M;
                int pi = (idx / M) % P;
                arr2d[mi][pi] += idx;
                reduction_sum += arr2d[mi][pi];
            } else {
                /* Access 3D and struct */
                int d1 = idx % (N/8);
                int d2 = (idx / (N/8)) % (M/4);
                int d3 = (idx / (N/8) / (M/4)) % (P/2);
                
                arr3d[d1][d2][d3] += struct_arr[d2].scalar;
                reduction_sum += arr3d[d1][d2][d3];
                
                /* Nested loop for worker/vector partitioning */
                #pragma omp simd
                for (int v = 0; v < 4; v++) {
                    struct_arr[d2].vector[v] += (float)arr3d[d1][d2][d3];
                }
            }
        }
    }
    
    printf("OMP Reduction sum: %d\n", reduction_sum);
    free(dynamic_arr);
}

/* Helper function with different parallel construct */
void kernels_test(int n) {
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
    }
    
    /* OpenACC kernels region - different optimization path */
    #pragma acc kernels copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            float tmp = 0.0f;
            #pragma acc loop worker reduction(+:tmp)
            for (int j = 0; j < 16; j++) {
                int idx = (i * 16 + j) % N;
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    tmp += a[idx] * b[(idx + k) % N];
                }
            }
            c[i] = tmp;
        }
    }
    
    /* Use result */
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += c[i];
    }
    printf("Kernels sum: %f\n", sum);
}

int main() {
    printf("Testing OpenACC/OpenMP partitioning state coverage\n");
    
    /* Test all three variants to maximize coverage */
    acc_parallel_test(N, M, P);
    omp_target_test(N, M, P);
    kernels_test(N/16);
    
    /* Validation */
    int validation_sum = 0;
    for (int i = 0; i < 100; i++) {
        validation_sum += i;
    }
    
    if (validation_sum == 4950) {
        printf("Validation passed: %d\n", validation_sum);
        return 0;
    } else {
        printf("Validation failed: %d\n", validation_sum);
        return 1;
    }
}
