/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in GCC's
 * omp-oacc-neuter-broadcast pass (cases 0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 16

/* Pattern A: Various scalar and array types with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Different array dimensions to trigger various partitioning */
    int scalar = 42;                     /* Likely gang redundant (0) */
    int arr1d[N];                        /* 1D array */
    int arr2d[M][N];                     /* 2D array */
    int arr3d[P][M][N];                  /* 3D array - complex partitioning */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            arr2d[i][j] = i * N + j;
        }
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < N; k++) {
                arr3d[i][j][k] = i * M * N + j * N + k;
            }
        }
    }
    
    /* Pattern B: Multi-dimensional array accesses with complex indexing */
    #pragma acc parallel loop copy(arr1d, arr2d, arr3d) copyin(scalar) \
        reduction(+:scalar) private(i, j, k) gang worker vector
    for (i = 0; i < P; i++) {
        /* Nested loops accessing different array dimensions */
        for (j = 0; j < M; j++) {
            for (k = 0; k < N; k++) {
                /* Complex conditional operations to create varied data flow */
                if (arr3d[i][j][k] % 2 == 0) {
                    arr1d[k] += arr2d[j][k];
                    arr3d[i][j][k] = arr1d[k] * 2;
                } else {
                    arr1d[k] -= arr2d[j][k] / 2;
                    arr3d[i][j][k] = arr1d[k] / 2;
                }
                
                /* Cross-dimensional access pattern */
                if (i > 0 && j > 0 && k > 0) {
                    arr2d[j][k] = arr3d[i-1][j-1][k-1] + arr3d[i][j][k];
                }
            }
        }
        
        /* Worker-level computation */
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            int worker_local = j * 10;
            #pragma acc loop vector
            for (k = 0; k < N; k++) {
                /* Vector-level computation with mixed access patterns */
                arr2d[j][k] += worker_local + k;
                arr1d[k] = arr2d[j][k] % 256;
            }
        }
    }
    
    /* Pattern C: Variable-length data and pointers */
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    int *dynamic_arr2 = (int*)malloc(M * N * sizeof(int));
    
    for (i = 0; i < N; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    for (i = 0; i < M * N; i++) {
        dynamic_arr2[i] = i * 3;
    }
    
    /* Map dynamic arrays with enter/exit data */
    #pragma acc enter data copyin(dynamic_arr[0:N], dynamic_arr2[0:M*N])
    
    #pragma acc parallel loop present(dynamic_arr, dynamic_arr2) \
        gang(32) worker(4) vector(32)
    for (i = 0; i < N; i++) {
        int idx = i % M;
        /* Pointer arithmetic and indirect access */
        dynamic_arr[i] = dynamic_arr2[idx * N + i] + dynamic_arr[i];
        
        /* Conditional with complex data dependencies */
        if (dynamic_arr[i] > 1000) {
            for (int w = 0; w < 4; w++) {
                dynamic_arr[i] -= w * dynamic_arr2[w * N + i];
            }
        }
    }
    
    #pragma acc exit data copyout(dynamic_arr[0:N]) delete(dynamic_arr2[0:M*N])
    
    free(dynamic_arr);
    free(dynamic_arr2);
    
    /* Pattern D: Struct/Class-like data (C struct) */
    typedef struct {
        int x;
        float y;
        double z;
        int arr[4];
    } ComplexData;
    
    ComplexData struct_arr[M];
    
    /* Initialize struct array */
    for (i = 0; i < M; i++) {
        struct_arr[i].x = i;
        struct_arr[i].y = i * 1.5f;
        struct_arr[i].z = i * 2.5;
        for (j = 0; j < 4; j++) {
            struct_arr[i].arr[j] = i * 4 + j;
        }
    }
    
    #pragma acc parallel loop copy(struct_arr) gang worker vector
    for (i = 0; i < M; i++) {
        /* Access different struct members in different contexts */
        struct_arr[i].x *= 2;
        
        #pragma acc loop seq
        for (j = 0; j < 4; j++) {
            struct_arr[i].arr[j] += struct_arr[i].x;
        }
        
        struct_arr[i].y = struct_arr[i].x * 0.5f;
        struct_arr[i].z = struct_arr[i].y * 2.0;
        
        /* Nested conditional with member access */
        if (i % 3 == 0) {
            for (int v = 0; v < 2; v++) {
                struct_arr[i].arr[v] = struct_arr[i].arr[v+2] * v;
            }
        }
    }
    
    /* Mixed OpenACC constructs to trigger different partitioning analyses */
    #pragma acc kernels copy(arr1d, arr2d) create(arr3d)
    {
        #pragma acc loop gang
        for (i = 0; i < P; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                #pragma acc loop vector
                for (k = 0; k < N; k++) {
                    /* Complex computation with mixed array accesses */
                    arr3d[i][j][k] = arr1d[k] + arr2d[j][k] * (i + 1);
                    
                    /* Reduction-like pattern */
                    if (k > 0) {
                        arr3d[i][j][k] += arr3d[i][j][k-1] / 2;
                    }
                }
            }
        }
    }
}

/* OpenMP version to cover both compilation paths */
void test_openmp_partitioning() {
    int i, j, k;
    int arr1d[N];
    int arr2d[M][N];
    double arr3d[P][M][N];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            arr2d[i][j] = i * N + j;
        }
    }
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr1d, arr2d) map(to: arr3d) \
        private(i, j, k) collapse(2)
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            /* Complex nested computation */
            #pragma omp simd
            for (k = 0; k < N; k++) {
                arr3d[i][j][k] = arr1d[k] * 0.5 + arr2d[j][k] * 2.0;
                
                /* Conditional with data-dependent branching */
                if (arr3d[i][j][k] > 100.0) {
                    arr1d[k] = (int)(arr3d[i][j][k] / 2.0);
                } else {
                    arr2d[j][k] = (int)(arr3d[i][j][k] * 2.0);
                }
            }
        }
    }
    
    /* Additional OpenMP construct with reduction */
    int sum = 0;
    #pragma omp target teams distribute parallel for \
        map(to: arr1d) reduction(+:sum) \
        private(i) shared(arr2d)
    for (i = 0; i < N; i++) {
        sum += arr1d[i];
        /* Cross-access pattern */
        if (i < M) {
            arr2d[i][i] = sum;
        }
    }
}

/* Main function with validation */
int main() {
    printf("Testing OpenACC partitioning states...\n");
    test_openacc_partitioning();
    
    printf("Testing OpenMP partitioning states...\n");
    test_openmp_partitioning();
    
    /* Simple validation to ensure code executes */
    int validation_arr[10] = {0};
    int expected_sum = 0;
    
    #pragma acc parallel loop copy(validation_arr)
    for (int i = 0; i < 10; i++) {
        validation_arr[i] = i * 2;
    }
    
    for (int i = 0; i < 10; i++) {
        expected_sum += i * 2;
    }
    
    int actual_sum = 0;
    for (int i = 0; i < 10; i++) {
        actual_sum += validation_arr[i];
    }
    
    if (actual_sum == expected_sum) {
        printf("Validation passed: sum = %d\n", actual_sum);
        return 0;
    } else {
        printf("Validation failed: expected %d, got %d\n", expected_sum, actual_sum);
        return 1;
    }
}
