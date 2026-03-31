/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in GCC's
 * omp-oacc-neuter-broadcast.cc switch statement (cases 0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#define USE_OPENACC 1
#elif _OPENMP
#define USE_OPENMP 1
#endif

#define N 128
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different dimensions */
void test_partitioning_states() {
    /* Different types of variables that may get different partitioning states */
    int scalar_private = 42;                    /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;              /* Likely gang redundant (0) */
    int reduction_sum = 0;                      /* Special reduction handling */
    
    /* 1D array - may be gang partitioned (1) */
    int arr1d[N];
    
    /* 2D array - may be worker partitioned (2) or gang+worker partitioned (3) */
    int arr2d[N][M];
    
    /* 3D array - complex partitioning possibilities */
    int arr3d[N][M][P];
    
    /* Pointer-based dynamic memory */
    int *dyn_arr = (int*)malloc(N * M * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i;
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = i * j;
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = i * j * k;
            }
        }
    }
    
    for (int i = 0; i < N * M; i++) {
        dyn_arr[i] = i % 17;
    }
    
    /* Pattern B: Multi-dimensional array accesses with complex indexing */
    #if USE_OPENACC
    #pragma acc parallel loop copy(arr1d[0:N]) \
        copyin(arr2d[0:N][0:M]) copyout(arr3d[0:N][0:M][0:P]) \
        copy(dyn_arr[0:N*M]) private(scalar_private) \
        firstprivate(scalar_firstprivate) reduction(+:reduction_sum) \
        gang worker vector
    #elif USE_OPENMP
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr1d[0:N]) \
        map(to: arr2d[0:N][0:M]) map(from: arr3d[0:N][0:M][0:P]) \
        map(tofrom: dyn_arr[0:N*M]) \
        private(scalar_private) firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum)
    #endif
    for (int i = 0; i < N; i++) {
        /* Pattern C: Complex nested loops and conditionals */
        int local_var = scalar_private + scalar_firstprivate;
        
        /* Access 1D array - potential gang partitioning */
        arr1d[i] += local_var;
        
        /* Nested loop for 2D array - potential worker partitioning */
        for (int j = 0; j < M; j++) {
            /* Conditional operation */
            if (arr2d[i][j] % 2 == 0) {
                arr2d[i][j] = arr2d[i][j] / 2;
            } else {
                arr2d[i][j] = arr2d[i][j] * 3 + 1;
            }
            
            /* Further nested loop for 3D array - potential vector partitioning */
            for (int k = 0; k < P; k++) {
                /* Pattern D: Complex indexing with all dimensions */
                int idx = (i * M * P + j * P + k) % (N * M * P);
                
                /* Access 3D array - potential gang+worker+vector partitioning */
                if (k % 4 == 0) {
                    arr3d[i][j][k] = arr3d[i][j][k] + idx;
                } else if (k % 4 == 1) {
                    arr3d[i][j][k] = arr3d[i][j][k] - idx;
                } else if (k % 4 == 2) {
                    arr3d[i][j][k] = arr3d[i][j][k] * 2;
                } else {
                    arr3d[i][j][k] = arr3d[i][j][k] / 2;
                }
                
                /* Access dynamic array */
                dyn_arr[idx % (N * M)] += arr3d[i][j][k] % 256;
                
                /* Reduction operation */
                reduction_sum += (arr3d[i][j][k] % 10);
            }
        }
        
        /* Additional conditional with scalar */
        if (i % 16 == 0) {
            scalar_private = (scalar_private * 3) % 97;
        }
    }
    
    /* Pattern E: Multiple parallel regions with different data mappings */
    int arr_small[10][20];
    int arr_medium[50][30][4];
    
    /* Initialize more arrays */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            arr_small[i][j] = i + j;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 30; j++) {
            for (int k = 0; k < 4; k++) {
                arr_medium[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Second parallel region with different characteristics */
    #if USE_OPENACC
    #pragma acc parallel loop gang(16) worker(8) vector_length(32) \
        copy(arr_small) create(arr_medium)
    #elif USE_OPENMP
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr_small) map(alloc: arr_medium)
    #endif
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            arr_small[i][j] = arr_small[i][j] * 2;
            
            /* Access medium array with different stride */
            if (i < 50 && j < 30) {
                for (int k = 0; k < 4; k++) {
                    arr_medium[i][j][k] = arr_small[i][j] + k;
                }
            }
        }
    }
    
    /* Pattern F: Struct/aggregate data type */
    struct DataPoint {
        int x;
        int y;
        float value;
        int metadata[4];
    };
    
    struct DataPoint data_array[100];
    
    /* Initialize struct array */
    for (int i = 0; i < 100; i++) {
        data_array[i].x = i;
        data_array[i].y = i * 2;
        data_array[i].value = i * 3.14f;
        for (int m = 0; m < 4; m++) {
            data_array[i].metadata[m] = i + m;
        }
    }
    
    /* Third parallel region with struct data */
    #if USE_OPENACC
    #pragma acc parallel loop copy(data_array)
    #elif USE_OPENMP
    #pragma omp target teams distribute parallel for map(tofrom: data_array)
    #endif
    for (int i = 0; i < 100; i++) {
        /* Access different struct members */
        data_array[i].x = data_array[i].x + 1;
        data_array[i].y = data_array[i].y * 2;
        data_array[i].value = data_array[i].value * 1.5f;
        
        /* Access struct array member */
        for (int m = 0; m < 4; m++) {
            data_array[i].metadata[m] = data_array[i].metadata[m] + i;
        }
    }
    
    /* Clean up */
    free(dyn_arr);
    
    /* Verification step to ensure code isn't dead */
    int verify_sum = 0;
    for (int i = 0; i < N; i++) {
        verify_sum = (verify_sum + arr1d[i]) % 1000;
    }
    printf("Verification sum: %d\n", verify_sum);
    printf("Reduction sum: %d\n", reduction_sum);
}

int main() {
    printf("Testing neuter-broadcast pass coverage...\n");
    
    test_partitioning_states();
    
    /* Additional test with different array sizes to trigger different optimizations */
    {
        int small_arr[8][8];
        int large_arr[256][128];
        
        /* Initialize */
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                small_arr[i][j] = i * j;
            }
        }
        
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 128; j++) {
                large_arr[i][j] = i + j;
            }
        }
        
        #if USE_OPENACC
        #pragma acc parallel loop copy(small_arr, large_arr)
        #elif USE_OPENMP
        #pragma omp target teams distribute parallel for \
            map(tofrom: small_arr, large_arr)
        #endif
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                small_arr[i][j] = small_arr[i][j] * 2;
            }
        }
        
        #if USE_OPENACC
        #pragma acc parallel loop copy(large_arr)
        #elif USE_OPENMP
        #pragma omp target teams distribute parallel for map(tofrom: large_arr)
        #endif
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 128; j++) {
                large_arr[i][j] = large_arr[i][j] + i - j;
            }
        }
    }
    
    printf("Test completed successfully.\n");
    return 0;
}
