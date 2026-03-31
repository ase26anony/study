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

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Different array dimensions to influence partitioning */
    int scalar = 42;                     /* Likely gang redundant (0) */
    int arr1d[N];                        /* 1D array */
    int arr2d[N][M];                     /* 2D array */
    int arr3d[N][M][P];                  /* 3D array - complex partitioning */
    int *dynamic_arr;                    /* Pointer to dynamic memory */
    int reduction_var = 0;               /* Reduction variable */
    
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
    
    /* Allocate and initialize dynamic array */
    dynamic_arr = (int *)malloc(N * M * sizeof(int));
    for (i = 0; i < N * M; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* OpenACC parallel region with complex data clauses and nested loops */
    #pragma acc parallel loop copy(arr1d, arr2d, arr3d) \
        copyin(scalar) copyout(dynamic_arr[0:N*M]) \
        reduction(+:reduction_var) \
        gang worker vector
    for (i = 0; i < N; i++) {
        /* Access scalar - may be gang redundant (0) */
        int local_scalar = scalar + i;
        
        /* Pattern B: Multi-dimensional array access with varying indices */
        for (j = 0; j < M; j++) {
            /* 2D array access - gang partitioned (1) or worker partitioned (2) */
            arr2d[i][j] += local_scalar;
            
            /* Pattern C: Complex conditional with pointer arithmetic */
            if (j % 2 == 0) {
                /* Access 3D array - gang+worker partitioned (3) */
                for (k = 0; k < P; k++) {
                    /* Vector partitioned (4) or gang+vector partitioned (5) */
                    arr3d[i][j][k] = arr3d[i][j][k] * 2 + k;
                    
                    /* Nested conditional for complex data flow */
                    if (k % 3 == 0) {
                        /* worker+vector partitioned (6) */
                        int temp = arr3d[i][j][k] / 2;
                        arr3d[i][j][k] = temp * 3;
                    }
                }
            } else {
                /* Different access pattern for odd j */
                for (k = 0; k < P; k += 2) {
                    /* fully partitioned (7) - all dimensions vary */
                    arr3d[i][j][k] = arr3d[i][j][k] + arr3d[i][j][k+1];
                }
            }
            
            /* Pattern D: Struct-like behavior using multiple arrays */
            int idx = i * M + j;
            if (idx < N * M) {
                /* Dynamic array access - different partitioning */
                dynamic_arr[idx] = arr2d[i][j] * 3;
            }
        }
        
        /* Reduction operation */
        reduction_var += arr1d[i];
        
        /* Complex loop with early exit to create varied live ranges */
        for (j = 0; j < M; j++) {
            if (arr2d[i][j] > 1000) {
                arr1d[i] += j;
                break;
            }
        }
    }
    
    /* Verify results to ensure code isn't dead */
    int checksum = 0;
    for (i = 0; i < N; i++) {
        checksum += arr1d[i] % 100;
    }
    printf("OpenACC checksum: %d\n", checksum);
    
    free(dynamic_arr);
}

/* Pattern E: OpenMP target region with teams/distribute */
void test_openmp_partitioning() {
    int i, j, k;
    
    /* Different variable types and storage classes */
    static int static_arr[N][M];         /* Static storage */
    int auto_arr[N][M];                  /* Automatic storage */
    volatile int volatile_var = 10;      /* Volatile variable */
    const int const_arr[N] = {[0 ... N-1] = 5}; /* Constant array */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            static_arr[i][j] = i + j;
            auto_arr[i][j] = i * j;
        }
    }
    
    /* OpenMP target region with complex mapping */
    #pragma omp target teams distribute parallel for \
        map(tofrom: static_arr, auto_arr) \
        map(to: const_arr) \
        private(i, j, k) \
        firstprivate(volatile_var) \
        collapse(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            /* Pattern F: Mixed scalar and array operations */
            int local_var = volatile_var + const_arr[i];
            
            /* Multiple array accesses with different dimensions */
            static_arr[i][j] = static_arr[i][j] * 2 + local_var;
            
            /* Nested loop inside parallel region */
            for (k = 0; k < P; k++) {
                /* Complex expression with multiple variables */
                auto_arr[i][j] += (k % 2 == 0) ? static_arr[i][j] : -static_arr[i][j];
                
                /* Conditional with function-like macro */
                #define PROCESS(x) ((x) * 3 + 1)
                if (k % 4 == 0) {
                    auto_arr[i][j] = PROCESS(auto_arr[i][j]);
                }
                #undef PROCESS
            }
            
            /* Pattern G: Pointer-like access using array decay */
            int *row_ptr = static_arr[i];
            for (k = 0; k < j; k++) {
                row_ptr[k] += auto_arr[i][j] % 10;
            }
        }
    }
    
    /* Another OpenMP region with different clauses */
    int reduction_sum = 0;
    #pragma omp target teams distribute parallel for \
        map(to: auto_arr) \
        reduction(+:reduction_sum) \
        num_teams(4) num_threads(32)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            reduction_sum += auto_arr[i][j] % 100;
            
            /* Pattern H: Switch statement inside parallel region */
            switch ((i + j) % 4) {
                case 0:
                    auto_arr[i][j] += 1;
                    break;
                case 1:
                    auto_arr[i][j] += 2;
                    break;
                case 2:
                    auto_arr[i][j] += 3;
                    break;
                case 3:
                    auto_arr[i][j] += 4;
                    break;
            }
        }
    }
    
    printf("OpenMP reduction sum: %d\n", reduction_sum);
}

/* Pattern I: Function with multiple parallel regions */
void mixed_partitioning_patterns() {
    int i, j;
    
    /* Arrays with different lifetimes and scopes */
    {
        int local_arr[N][M];
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                local_arr[i][j] = (i << 2) + j;
            }
        }
        
        /* OpenACC kernels region (different from parallel) */
        #pragma acc kernels copy(local_arr)
        {
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    /* Bitwise operations for varied patterns */
                    local_arr[i][j] = (local_arr[i][j] & 0xFF) | 
                                     ((local_arr[i][j] << 8) & 0xFF00);
                    
                    /* Complex conditional chain */
                    if (local_arr[i][j] < 100) {
                        local_arr[i][j] *= 2;
                    } else if (local_arr[i][j] < 200) {
                        local_arr[i][j] /= 2;
                    } else {
                        local_arr[i][j] = local_arr[i][j] % 100;
                    }
                }
            }
        }
    }
    
    /* Pattern J: Recursive-like pattern using multiple functions */
    {
        int arr_chain[4][N];
        for (i = 0; i < 4; i++) {
            for (j = 0; j < N; j++) {
                arr_chain[i][j] = i * N + j;
            }
        }
        
        /* Multiple nested parallel regions */
        #pragma acc parallel loop copy(arr_chain)
        for (i = 0; i < 4; i++) {
            #pragma acc loop
            for (j = 0; j < N; j++) {
                /* Access pattern that varies across multiple dimensions */
                arr_chain[i][j] = arr_chain[(i + 1) % 4][j] + 
                                 arr_chain[i][(j + 1) % N];
            }
        }
    }
}

/* Main function that runs all tests */
int main() {
    printf("Testing OpenACC/OpenMP neuter-broadcast pass coverage...\n");
    
    /* Run OpenACC test */
    test_openacc_partitioning();
    
    /* Run OpenMP test */
    test_openmp_partitioning();
    
    /* Run mixed patterns test */
    mixed_partitioning_patterns();
    
    printf("All tests completed.\n");
    
    /* Return success */
    return 0;
}
