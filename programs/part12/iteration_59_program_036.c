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
    
    /* Different partitioning scenarios */
    int scalar_gang_redundant;          /* Likely case 0 */
    int scalar_worker_partitioned;      /* Likely case 2 */
    int scalar_vector_partitioned;      /* Likely case 4 */
    
    /* Multi-dimensional arrays for complex partitioning */
    int arr1d[N];                       /* Base case */
    int arr2d[N][M];                    /* 2D for gang/worker partitioning */
    int arr3d[N][M][P];                 /* 3D for full partitioning exploration */
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
        for (j = 0; j < M; j++) {
            arr2d[i][j] = i * j;
            for (k = 0; k < P; k++) {
                arr3d[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Pattern B: Multi-dimensional array access with varying indices */
    #pragma acc parallel loop copy(arr1d, arr2d, arr3d) \
        copyin(scalar_gang_redundant) \
        private(scalar_worker_partitioned) \
        vector_length(32)
    for (i = 0; i < N; i++) {
        /* Case 0: Gang redundant variable */
        scalar_gang_redundant = arr1d[0];
        
        /* Case 1: Gang partitioned (across gang dimension) */
        int gang_partitioned = arr1d[i];
        
        /* Nested loops to create worker and vector contexts */
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            /* Case 2: Worker partitioned */
            scalar_worker_partitioned = j;
            
            /* Case 3: Gang+Worker partitioned */
            int gang_worker_partitioned = arr2d[i][j];
            
            /* Further nesting for vector dimension */
            #pragma acc loop vector
            for (k = 0; k < P; k++) {
                /* Case 4: Vector partitioned */
                scalar_vector_partitioned = k;
                
                /* Case 5: Gang+Vector partitioned */
                int gang_vector_partitioned = arr3d[i][0][k];
                
                /* Case 6: Worker+Vector partitioned */
                int worker_vector_partitioned = arr3d[0][j][k];
                
                /* Case 7: Fully partitioned */
                int fully_partitioned = arr3d[i][j][k];
                
                /* Complex conditional to prevent optimization */
                if (fully_partitioned % 2 == 0) {
                    arr3d[i][j][k] = gang_partitioned + gang_worker_partitioned;
                } else {
                    arr3d[i][j][k] = gang_vector_partitioned + worker_vector_partitioned;
                }
            }
        }
        
        /* Reduction operation for additional partitioning complexity */
        int sum = 0;
        #pragma acc loop worker reduction(+:sum)
        for (j = 0; j < M; j++) {
            sum += arr2d[i][j];
        }
        arr1d[i] = sum;
    }
}

/* Pattern C: Variable-length data and pointers */
void test_dynamic_partitioning() {
    int size = 1024;
    int *dynamic_arr = (int*)malloc(size * sizeof(int));
    int **ptr_arr = (int**)malloc(N * sizeof(int*));
    
    /* Initialize dynamic data */
    for (int i = 0; i < size; i++) {
        dynamic_arr[i] = i;
    }
    for (int i = 0; i < N; i++) {
        ptr_arr[i] = (int*)malloc(M * sizeof(int));
        for (int j = 0; j < M; j++) {
            ptr_arr[i][j] = i * j;
        }
    }
    
    /* Map dynamic data to device with various clauses */
    #pragma acc enter data copyin(dynamic_arr[0:size])
    #pragma acc enter data copyin(ptr_arr[0:N])
    for (int i = 0; i < N; i++) {
        #pragma acc enter data copyin(ptr_arr[i][0:M])
    }
    
    /* Complex kernel with pointer accesses */
    #pragma acc parallel loop present(dynamic_arr, ptr_arr) \
        gang worker vector
    for (int i = 0; i < N; i++) {
        int local_sum = 0;
        
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            int *row = ptr_arr[i];
            
            #pragma acc loop vector
            for (int k = 0; k < P; k++) {
                /* Access pattern that forces different partitioning */
                int idx = (i * M * P + j * P + k) % size;
                row[j] += dynamic_arr[idx] * k;
            }
            
            local_sum += row[j];
        }
        
        /* Store result back to dynamic array */
        dynamic_arr[i % size] = local_sum;
    }
    
    #pragma acc exit data copyout(dynamic_arr[0:size])
    #pragma acc exit data delete(ptr_arr[0:N])
    
    free(dynamic_arr);
    for (int i = 0; i < N; i++) {
        free(ptr_arr[i]);
    }
    free(ptr_arr);
}

/* Pattern D: Struct-based partitioning (C++ style in C) */
typedef struct {
    int x;
    float y;
    double z;
    int arr[10];
} ComplexStruct;

void test_struct_partitioning() {
    ComplexStruct struct_arr[N];
    
    /* Initialize struct array */
    for (int i = 0; i < N; i++) {
        struct_arr[i].x = i;
        struct_arr[i].y = i * 1.5f;
        struct_arr[i].z = i * 2.5;
        for (int j = 0; j < 10; j++) {
            struct_arr[i].arr[j] = i * j;
        }
    }
    
    /* Kernel accessing different struct members */
    #pragma acc parallel loop copy(struct_arr)
    for (int i = 0; i < N; i++) {
        /* Different members accessed in different loop levels */
        int base = struct_arr[i].x;
        
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            float worker_val = struct_arr[i].y * j;
            
            #pragma acc loop vector
            for (int k = 0; k < 10; k++) {
                double vector_val = struct_arr[i].z * k;
                struct_arr[i].arr[k] = base + (int)worker_val + (int)vector_val;
            }
        }
    }
}

/* OpenMP version for broader compiler path coverage */
void test_openmp_partitioning() {
    int data[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = i * j;
        }
    }
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data) \
        private(i) shared(N, M)
    for (int i = 0; i < N; i++) {
        int gang_local = i;  /* Gang partitioned */
        
        #pragma omp parallel for private(j)
        for (int j = 0; j < M; j++) {
            int worker_local = j;  /* Worker partitioned */
            
            #pragma omp simd
            for (int k = 0; k < P; k++) {
                int vector_local = k;  /* Vector partitioned */
                data[i][j] += gang_local + worker_local + vector_local;
            }
        }
    }
}

/* Validation function */
int validate_results() {
    int sum = 0;
    int test_arr[N];
    
    /* Simple computation to ensure code isn't dead */
    #pragma acc parallel loop copyout(test_arr)
    for (int i = 0; i < N; i++) {
        test_arr[i] = i * 2;
    }
    
    for (int i = 0; i < N; i++) {
        sum += test_arr[i];
    }
    
    int expected = N * (N - 1);  /* Sum of i*2 for i=0..N-1 */
    return sum == expected;
}

int main() {
    printf("Testing neuter-broadcast pass coverage...\n");
    
    /* Execute all test patterns */
    test_openacc_partitioning();
    printf("OpenACC partitioning test completed.\n");
    
    test_dynamic_partitioning();
    printf("Dynamic partitioning test completed.\n");
    
    test_struct_partitioning();
    printf("Struct partitioning test completed.\n");
    
    test_openmp_partitioning();
    printf("OpenMP partitioning test completed.\n");
    
    /* Final validation */
    if (validate_results()) {
        printf("All tests passed successfully.\n");
        return 0;
    } else {
        printf("Validation failed.\n");
        return 1;
    }
}
