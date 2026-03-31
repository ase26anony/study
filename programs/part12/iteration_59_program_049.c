/* test_neuter_broadcast.c
 * Comprehensive test to trigger all partitioning states in GCC's
 * omp-oacc-neuter-broadcast pass (cases 0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Pattern A: Various scalar and array variables with different data clauses */
void test_openacc_partitioning(int *results) {
    int i, j, k;
    
    /* Static arrays with different dimensions */
    int arr1d[N];                    /* 1D array */
    int arr2d[N][M];                 /* 2D array */
    int arr3d[N][M][P];              /* 3D array - complex partitioning */
    
    /* Scalars with different storage classes */
    int scalar_private;
    int scalar_firstprivate = 42;
    int scalar_reduction = 0;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
        for (j = 0; j < M; j++) {
            arr2d[i][j] = i * j;
            for (k = 0; k < P; k++) {
                arr3d[i][j][k] = i * j * k;
            }
        }
    }
    
    /* OpenACC parallel region with complex data clauses and nesting */
    #pragma acc parallel loop copy(arr1d, arr2d, arr3d) \
        copyin(scalar_firstprivate) copyout(results[0:N*M]) \
        private(scalar_private) reduction(+:scalar_reduction) \
        gang vector
    for (i = 0; i < N; i++) {
        scalar_private = i;  /* Private to each gang/worker/vector */
        
        /* Nested loops accessing multi-dimensional arrays */
        for (j = 0; j < M; j++) {
            /* Conditional operations create complex data flow */
            if (arr2d[i][j] % 2 == 0) {
                /* Access 3D array - triggers complex partitioning analysis */
                for (k = 0; k < P; k++) {
                    scalar_reduction += arr3d[i][j][k] % 7;
                }
                
                /* Write to 1D array with stride */
                arr1d[(i * M + j) % N] = arr2d[i][j] + scalar_firstprivate;
            } else {
                /* Different access pattern for odd elements */
                arr2d[i][j] = arr2d[i][j] * 2;
                
                /* Conditional store to results array */
                results[i * M + j] = arr2d[i][j] + scalar_private;
            }
            
            /* Vector-level operation */
            #pragma acc loop vector
            for (k = 0; k < 8; k++) {
                arr3d[i][j][k % P] += k;
            }
        }
        
        /* Worker-level operation */
        #pragma acc loop worker
        for (j = 0; j < 4; j++) {
            int worker_temp = scalar_private + j;
            arr1d[(i + j) % N] += worker_temp;
        }
    }
    
    /* Additional kernel with different partitioning */
    #pragma acc kernels copy(arr1d, arr2d) create(scalar_private)
    {
        scalar_private = 0;
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            #pragma acc loop vector
            for (j = 0; j < M; j++) {
                arr2d[i][j] += scalar_private;
                scalar_private = (scalar_private + 1) % 100;
            }
        }
    }
}

/* Pattern B: Dynamic memory and pointers */
void test_dynamic_memory_partitioning() {
    int i, j;
    
    /* Dynamic arrays */
    int *dyn_arr1d = (int *)malloc(N * M * sizeof(int));
    int **dyn_arr2d = (int **)malloc(N * sizeof(int *));
    
    for (i = 0; i < N; i++) {
        dyn_arr2d[i] = (int *)malloc(M * sizeof(int));
        for (j = 0; j < M; j++) {
            dyn_arr2d[i][j] = i * 100 + j;
        }
    }
    
    /* OpenACC with dynamic data */
    #pragma acc enter data copyin(dyn_arr1d[0:N*M])
    #pragma acc enter data copyin(dyn_arr2d[0:N])
    for (i = 0; i < N; i++) {
        #pragma acc enter data copyin(dyn_arr2d[i][0:M])
    }
    
    #pragma acc parallel loop present(dyn_arr1d, dyn_arr2d) gang worker vector
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            dyn_arr1d[i * M + j] = dyn_arr2d[i][j] * 2;
            
            /* Complex conditional with pointer arithmetic */
            if ((i + j) % 3 == 0) {
                dyn_arr2d[i][j] = dyn_arr1d[i * M + j] / 2;
            } else if ((i + j) % 3 == 1) {
                dyn_arr2d[i][j] = dyn_arr1d[i * M + j] * 3;
            }
        }
    }
    
    #pragma acc exit data copyout(dyn_arr1d[0:N*M])
    #pragma acc exit data copyout(dyn_arr2d[0:N])
    for (i = 0; i < N; i++) {
        #pragma acc exit data copyout(dyn_arr2d[i][0:M])
    }
    
    /* Cleanup */
    for (i = 0; i < N; i++) {
        free(dyn_arr2d[i]);
    }
    free(dyn_arr2d);
    free(dyn_arr1d);
}

/* Pattern C: OpenMP target regions for comparison */
void test_openmp_partitioning(int *arr) {
    int i, j;
    
    /* Multi-dimensional static array */
    int local_arr[N][M];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            local_arr[i][j] = i * M + j;
        }
    }
    
    /* OpenMP target with teams and distribute - triggers different partitioning */
    #pragma omp target teams distribute parallel for \
        map(tofrom: local_arr) map(to: arr[0:N*M]) \
        private(i, j) collapse(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            /* Complex computation with conditionals */
            if (local_arr[i][j] % 4 == 0) {
                local_arr[i][j] = arr[i * M + j] + local_arr[i][j];
            } else if (local_arr[i][j] % 4 == 1) {
                local_arr[i][j] = arr[i * M + j] * local_arr[i][j];
            } else {
                local_arr[i][j] = arr[i * M + j] - local_arr[i][j];
            }
            
            /* Nested loop inside parallel region */
            int k;
            for (k = 0; k < 4; k++) {
                local_arr[i][j] += k;
            }
        }
    }
    
    /* Another OpenMP region with different data mapping */
    int reduction_var = 0;
    #pragma omp target teams distribute parallel for \
        map(tofrom: local_arr) reduction(+:reduction_var) \
        num_teams(4) num_threads(32)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            reduction_var += local_arr[i][j] % 17;
            
            /* Vector-like operation simulated */
            #pragma omp simd
            for (int k = 0; k < 8; k++) {
                local_arr[i][j] += (k % 3);
            }
        }
    }
}

/* Pattern D: Struct/aggregate data type */
struct DataPoint {
    int x;
    int y;
    float value;
    int metadata[4];
};

void test_struct_partitioning() {
    int i;
    struct DataPoint data[N];
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        data[i].x = i;
        data[i].y = i * 2;
        data[i].value = i * 3.14f;
        for (int j = 0; j < 4; j++) {
            data[i].metadata[j] = i * 10 + j;
        }
    }
    
    /* OpenACC with struct array - members may have different partitioning */
    #pragma acc parallel loop copy(data) gang worker vector
    for (i = 0; i < N; i++) {
        /* Different operations on different struct members */
        data[i].x = data[i].x * 2;
        data[i].y = data[i].y + data[i].x;
        data[i].value = data[i].value * 1.5f;
        
        /* Access metadata array within struct */
        for (int j = 0; j < 4; j++) {
            if (j % 2 == 0) {
                data[i].metadata[j] = data[i].x + j;
            } else {
                data[i].metadata[j] = data[i].y - j;
            }
        }
        
        /* Conditional with complex expression */
        if (data[i].value > 50.0f) {
            data[i].x = data[i].x / 2;
            #pragma acc loop seq
            for (int j = 0; j < 2; j++) {
                data[i].metadata[j] = data[i].metadata[j] * 3;
            }
        }
    }
}

/* Main function that exercises all patterns */
int main() {
    int i, j;
    int results[N * M];
    int input_arr[N * M];
    
    printf("Testing OpenACC/OpenMP neuter-broadcast partitioning states\n");
    
    /* Initialize input arrays */
    for (i = 0; i < N * M; i++) {
        input_arr[i] = i;
        results[i] = 0;
    }
    
    /* Test 1: OpenACC with various data types and clauses */
    printf("Running OpenACC partitioning test...\n");
    test_openacc_partitioning(results);
    
    /* Verify results */
    int sum = 0;
    for (i = 0; i < N * M; i++) {
        sum += results[i];
    }
    printf("OpenACC result checksum: %d\n", sum);
    
    /* Test 2: Dynamic memory partitioning */
    printf("Running dynamic memory partitioning test...\n");
    test_dynamic_memory_partitioning();
    
    /* Test 3: OpenMP target regions */
    printf("Running OpenMP partitioning test...\n");
    test_openmp_partitioning(input_arr);
    
    /* Test 4: Struct/aggregate data */
    printf("Running struct partitioning test...\n");
    test_struct_partitioning();
    
    /* Final verification */
    printf("All tests completed successfully.\n");
    
    return 0;
}
