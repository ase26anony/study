/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in GCC's omp-oacc-neuter-broadcast pass
 * Compile with: gcc -O2 -fopenacc -foffload=disable -fdump-tree-all -fprofile-arcs -ftest-coverage test_neuter_broadcast.c -o test_neuter_broadcast
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Pattern A: Mix of scalar variables and arrays */
void test_pattern_a(int *result) {
    int scalar_private = 42;          /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;    /* Likely gang redundant (0) */
    int reduction_sum = 0;            /* Reduction variable */
    int arr1d[N];                     /* 1D array */
    int arr2d[M][M];                  /* 2D array */
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = i * M + j;
        }
    }
    
    /* OpenACC parallel region with diverse data clauses */
    #pragma acc parallel loop copy(arr1d[0:N]) copyin(arr2d) \
        private(scalar_private) firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        /* Complex conditional operations to create varied data flow */
        if (i % 2 == 0) {
            scalar_private = arr1d[i] * 2;
            result[i] = scalar_private + scalar_firstprivate;
        } else {
            scalar_private = arr1d[i] / 2;
            result[i] = scalar_private - scalar_firstprivate;
        }
        
        /* Nested loop accessing 2D array */
        int temp = 0;
        for (int j = 0; j < M; j++) {
            if (j < i % M) {
                temp += arr2d[i % M][j];
            }
        }
        result[i] += temp % 100;
        
        reduction_sum += result[i];
    }
    
    result[0] = reduction_sum;
}

/* Pattern B: Multi-dimensional arrays with complex access patterns */
void test_pattern_b(int *result) {
    int arr3d[P][M][M];  /* 3D array - likely to trigger various partitioning states */
    
    /* Initialize 3D array */
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < M; k++) {
                arr3d[i][j][k] = i * M * M + j * M + k;
            }
        }
    }
    
    /* OpenACC kernels region - different construct may trigger different analysis */
    #pragma acc kernels copyin(arr3d) copy(result[0:P*M])
    {
        #pragma acc loop gang
        for (int i = 0; i < P; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < M; k++) {
                    /* Access varying across all three dimensions */
                    int idx = i * M * M + j * M + k;
                    if (idx < N) {
                        /* Complex conditional with multi-dimensional access */
                        if ((i + j + k) % 3 == 0) {
                            result[idx] = arr3d[i][j][k] * 2;
                        } else if ((i + j + k) % 3 == 1) {
                            result[idx] = arr3d[i][j][k] + arr3d[(i+1)%P][j][k];
                        } else {
                            result[idx] = arr3d[i][j][k] - arr3d[i][(j+1)%M][k];
                        }
                    }
                }
            }
        }
    }
}

/* Pattern C: Variable-length data and pointers */
void test_pattern_c(int *result) {
    int *dynamic_arr;
    int static_arr[N];
    int *ptr_arr[M];
    
    /* Dynamic allocation */
    dynamic_arr = (int *)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        static_arr[i] = i * 3;
        dynamic_arr[i] = i * 5;
    }
    
    for (int i = 0; i < M; i++) {
        ptr_arr[i] = &static_arr[i * (N/M)];
    }
    
    /* OpenACC with pointer-based access */
    #pragma acc parallel loop copyin(static_arr[0:N]) \
        copyin(dynamic_arr[0:N]) copy(result[0:N]) \
        copyin(ptr_arr[0:M])
    for (int i = 0; i < N; i++) {
        /* Access through different pointer types */
        int base = static_arr[i];
        int dyn_val = dynamic_arr[(i * 7) % N];
        
        /* Conditional pointer dereference */
        if (i < M) {
            base += *ptr_arr[i % M];
        }
        
        /* Nested loop with pointer arithmetic */
        int sum = 0;
        for (int j = 0; j < 10; j++) {
            if ((i + j) < N) {
                sum += dynamic_arr[i + j];
            }
        }
        
        result[i] = base + dyn_val + sum;
    }
    
    free(dynamic_arr);
}

/* Pattern D: Struct-based data (C structs) */
struct DataPoint {
    int x;
    int y;
    int z;
    float value;
};

void test_pattern_d(int *result) {
    struct DataPoint points[N];
    struct DataPoint local_copy;
    
    /* Initialize struct array */
    for (int i = 0; i < N; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
        points[i].value = i * 1.5f;
    }
    
    local_copy.x = 100;
    local_copy.y = 200;
    local_copy.z = 300;
    local_copy.value = 400.0f;
    
    /* OpenACC with struct array */
    #pragma acc parallel loop copy(points[0:N]) \
        firstprivate(local_copy) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        /* Access different struct members with different patterns */
        int temp = points[i].x + points[i].y;
        
        /* Conditional struct member access */
        if (i % 4 == 0) {
            temp += points[i].z;
            points[i].value += local_copy.value;
        } else if (i % 4 == 1) {
            temp -= local_copy.x;
            points[i].value *= 2.0f;
        } else if (i % 4 == 2) {
            temp += points[(i + 1) % N].x;
            points[i].value = local_copy.value;
        } else {
            temp = points[i].y * points[i].z;
            points[i].value /= 2.0f;
        }
        
        /* Nested loop accessing struct array */
        int sum = 0;
        for (int j = 0; j < 5; j++) {
            if (i + j < N) {
                sum += points[i + j].x;
            }
        }
        
        result[i] = temp + sum + (int)points[i].value;
    }
}

/* OpenMP version to ensure both OpenACC and OpenMP paths are tested */
#ifdef _OPENMP
void test_omp_pattern(int *result) {
    int arr[N];
    int local_var = 50;
    int reduction_var = 0;
    
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(to: arr[0:N]) map(from: result[0:N]) \
        firstprivate(local_var) reduction(+:reduction_var)
    for (int i = 0; i < N; i++) {
        /* Complex nested conditional */
        int temp = arr[i];
        for (int j = 0; j < 8; j++) {
            if ((i + j) % 8 == j) {
                temp += local_var * j;
            }
        }
        
        if (i % 3 == 0) {
            result[i] = temp * 2;
        } else if (i % 3 == 1) {
            result[i] = temp + local_var;
        } else {
            result[i] = temp - local_var;
        }
        
        reduction_var += result[i];
    }
    
    result[0] = reduction_var;
}
#endif

/* Main function that runs all patterns */
int main() {
    int *results[4];
    int checksum = 0;
    
    printf("Testing neuter-broadcast pass coverage...\n");
    
    /* Allocate result arrays */
    for (int i = 0; i < 4; i++) {
        results[i] = (int *)malloc(N * sizeof(int));
        memset(results[i], 0, N * sizeof(int));
    }
    
    /* Run all test patterns */
    test_pattern_a(results[0]);
    test_pattern_b(results[1]);
    test_pattern_c(results[2]);
    test_pattern_d(results[3]);
    
    #ifdef _OPENMP
    test_omp_pattern(results[0]);  /* Reuse first array for OMP */
    #endif
    
    /* Compute checksum to ensure code executes */
    for (int p = 0; p < 4; p++) {
        for (int i = 0; i < N && i < 100; i++) {  /* Check first 100 elements */
            checksum += results[p][i];
            checksum %= 1000000;
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(results[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
