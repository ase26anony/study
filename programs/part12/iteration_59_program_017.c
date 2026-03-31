/* Test for omp-oacc-neuter-broadcast.cc coverage of partitioning state strings */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 50
#define P 20

/* Pattern A: Different variable types with various data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Create variables that should get different partitioning states */
    int scalar_private;                     /* Likely gang redundant (0) */
    int scalar_firstprivate = 42;           /* Likely gang redundant (0) */
    int reduction_sum = 0;                  /* Special reduction handling */
    
    /* Multi-dimensional arrays with different access patterns */
    int arr1d[N];                           /* 1D array */
    int arr2d[N][M];                        /* 2D array */
    int arr3d[N][M][P];                     /* 3D array - complex partitioning */
    
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
    
    /* Pattern B: Multi-dimensional array access with nested loops */
    #pragma acc parallel loop gang worker vector \
        copy(arr1d, arr2d, arr3d) \
        copyin(scalar_firstprivate) \
        private(scalar_private) \
        reduction(+:reduction_sum)
    for (i = 0; i < N; i++) {
        scalar_private = i;  /* Private to each thread */
        
        /* Worker-level computation */
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            int worker_local = scalar_firstprivate + j;
            
            /* Vector-level computation */
            #pragma acc loop vector
            for (k = 0; k < P; k++) {
                /* Complex access pattern to trigger different partitioning */
                arr3d[i][j][k] += worker_local + k;
                
                /* Conditional to create complex data flow */
                if (k % 2 == 0) {
                    arr2d[i][j] += arr3d[i][j][k] % 7;
                } else {
                    arr2d[i][j] -= arr3d[i][j][k] % 5;
                }
            }
            
            /* Reduction across workers */
            reduction_sum += arr2d[i][j];
        }
        
        /* Update 1D array with gang-specific computation */
        arr1d[i] = scalar_private * scalar_firstprivate;
    }
    
    /* Pattern C: Dynamic memory with pointer-based access */
    int *dynamic_arr = (int*)malloc(N * M * sizeof(int));
    int **ptr_array = (int**)malloc(N * sizeof(int*));
    
    /* Initialize dynamic structures */
    for (i = 0; i < N; i++) {
        ptr_array[i] = &dynamic_arr[i * M];
        for (j = 0; j < M; j++) {
            ptr_array[i][j] = i + j;
        }
    }
    
    /* Map dynamic memory to device */
    #pragma acc enter data copyin(dynamic_arr[0:N*M], ptr_array[0:N])
    
    /* Another parallel region with pointer access */
    #pragma acc parallel loop gang \
        present(dynamic_arr, ptr_array)
    for (i = 0; i < N; i++) {
        int gang_local = 0;
        
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            gang_local += ptr_array[i][j];
            
            #pragma acc loop vector
            for (k = 0; k < 10; k++) {
                dynamic_arr[i * M + j] += (gang_local + k) % 11;
            }
        }
    }
    
    #pragma acc exit data delete(dynamic_arr, ptr_array)
    
    free(dynamic_arr);
    free(ptr_array);
    
    /* Verify results to ensure code isn't dead */
    int checksum = 0;
    for (i = 0; i < N; i++) {
        checksum = (checksum + arr1d[i]) % 1000;
        for (j = 0; j < M; j++) {
            checksum = (checksum + arr2d[i][j]) % 1000;
        }
    }
    printf("OpenACC checksum: %d\n", checksum);
}

/* Pattern D: C struct for complex data layout */
struct ComplexData {
    int x;
    float y;
    double z;
    int arr[5];
};

void test_openmp_partitioning() {
    int i, j;
    
    /* Array of structs */
    struct ComplexData data_array[N];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        data_array[i].x = i;
        data_array[i].y = i * 0.5f;
        data_array[i].z = i * 0.25;
        for (j = 0; j < 5; j++) {
            data_array[i].arr[j] = i * j;
        }
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data_array) \
        num_teams(4) num_threads(32)
    for (i = 0; i < N; i++) {
        /* Access different struct members with different patterns */
        data_array[i].x *= 2;           /* Simple gang-level access */
        
        #pragma omp simd
        for (j = 0; j < 5; j++) {
            /* Vector-level access to array inside struct */
            data_array[i].arr[j] += data_array[i].x;
            
            /* Conditional to create complex control flow */
            if (data_array[i].arr[j] % 3 == 0) {
                data_array[i].y += 1.0f;
            } else {
                data_array[i].z -= 0.5;
            }
        }
    }
    
    /* Nested parallel regions for additional partitioning states */
    int shared_var = 0;
    int private_var;
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: shared_var) private(private_var)
    for (i = 0; i < N; i++) {
        private_var = i;
        
        /* Complex reduction-like pattern */
        #pragma omp atomic
        shared_var += private_var;
        
        /* Multi-level nested loops */
        for (int worker_idx = 0; worker_idx < 4; worker_idx++) {
            int worker_local = private_var + worker_idx;
            
            for (int vector_idx = 0; vector_idx < 8; vector_idx++) {
                data_array[i].arr[vector_idx % 5] += worker_local + vector_idx;
            }
        }
    }
    
    /* Verify results */
    double total = 0.0;
    for (i = 0; i < N; i++) {
        total += data_array[i].x + data_array[i].y + data_array[i].z;
        for (j = 0; j < 5; j++) {
            total += data_array[i].arr[j];
        }
    }
    printf("OpenMP total: %.2f\n", total);
}

/* Additional test with mixed constructs */
void test_mixed_partitioning() {
    int i, j, k;
    int mixed_arr[N][M][P];
    
    /* Initialize with pattern */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                mixed_arr[i][j][k] = (i * 1000) + (j * 100) + k;
            }
        }
    }
    
    /* Combined gang-worker-vector partitioning */
    #pragma acc parallel loop gang(4) worker(2) vector_length(32) \
        copy(mixed_arr)
    for (i = 0; i < N; i++) {
        int gang_var = i * 10;  /* Gang-level variable */
        
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            int worker_var = gang_var + j;  /* Worker-level variable */
            
            #pragma acc loop vector
            for (k = 0; k < P; k++) {
                /* This creates complex data dependencies across
                   gang, worker, and vector dimensions */
                if (k % 2 == 0) {
                    mixed_arr[i][j][k] += worker_var;
                } else {
                    mixed_arr[i][j][k] -= worker_var;
                }
                
                /* Cross-dimensional access pattern */
                if (j > 0 && k > 0) {
                    mixed_arr[i][j][k] += mixed_arr[i][j-1][k-1] % 17;
                }
            }
        }
    }
    
    /* Final checksum */
    long long final_sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                final_sum += mixed_arr[i][j][k];
            }
        }
    }
    printf("Mixed final sum: %lld\n", final_sum);
}

int main() {
    printf("Testing OpenACC partitioning states...\n");
    test_openacc_partitioning();
    
    printf("Testing OpenMP partitioning states...\n");
    test_openmp_partitioning();
    
    printf("Testing mixed partitioning states...\n");
    test_mixed_partitioning();
    
    printf("All tests completed.\n");
    return 0;
}
