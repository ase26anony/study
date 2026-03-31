/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    int scalar_private = 42;           /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;     /* Likely gang redundant (0) */
    int reduction_sum = 0;             /* Reduction variable */
    
    /* Static arrays with different dimensions */
    int arr1d[N];                      /* 1D array */
    int arr2d[N][M];                   /* 2D array */
    int arr3d[N][M][P];                /* 3D array - complex partitioning */
    
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
    
    /* Pattern B: Multi-dimensional array accesses with complex indexing */
    #pragma acc parallel loop gang worker vector \
        copy(arr1d[0:N], arr2d[0:N][0:M]) \
        copyin(arr3d[0:N][0:M][0:P]) \
        copyout(reduction_sum) \
        private(scalar_private) \
        firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum)
    for (i = 0; i < N; i++) {
        /* Access 1D array - may be gang partitioned (1) */
        int local_val = arr1d[i];
        
        /* Nested loops create worker/vector partitioning opportunities */
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            /* Access 2D array - may be worker partitioned (2) or gang+worker (3) */
            int temp = arr2d[i][j];
            
            /* Pattern C: Pointer-like access through multi-dim array */
            #pragma acc loop vector
            for (k = 0; k < P; k++) {
                /* Access 3D array - may be vector partitioned (4) or combinations (5,6,7) */
                int val3d = arr3d[i][j][k];
                
                /* Complex conditional creates varied data flow */
                if (val3d % 2 == 0) {
                    local_val += temp;
                } else {
                    local_val -= val3d % 7;
                }
                
                /* Reduction operation */
                reduction_sum += (val3d > 1000) ? 1 : 0;
            }
            
            /* Cross-dimensional access pattern */
            if (j > 0) {
                arr2d[i][j] = arr2d[i][j-1] + local_val;
            }
        }
        
        /* Write back with stride pattern */
        arr1d[i] = local_val * (i % 8 + 1);
    }
    
    printf("OpenACC reduction sum: %d\n", reduction_sum);
}

/* Pattern D: Struct-based data for complex partitioning analysis */
struct DataPoint {
    int x;
    int y;
    float value;
    int metadata[4];
};

void test_openmp_partitioning() {
    int i, j;
    struct DataPoint data[N];
    int partial_sums[M];
    int* dynamic_array;
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        data[i].x = i;
        data[i].y = i * 2;
        data[i].value = i * 3.14f;
        for (j = 0; j < 4; j++) {
            data[i].metadata[j] = i * j;
        }
    }
    
    /* Pattern C: Dynamic memory */
    dynamic_array = (int*)malloc(N * M * sizeof(int));
    for (i = 0; i < N * M; i++) {
        dynamic_array[i] = i % 97;
    }
    
    /* Initialize partial sums */
    for (i = 0; i < M; i++) {
        partial_sums[i] = 0;
    }
    
    /* OpenMP target with teams and distribute - triggers different partitioning */
    #pragma omp target teams distribute parallel for \
        map(to: data[0:N]) \
        map(tofrom: partial_sums[0:M]) \
        map(to: dynamic_array[0:N*M])
    for (i = 0; i < N; i++) {
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        
        /* Access struct members - different partitioning for different fields */
        int base = data[i].x + data[i].y;
        float scaled = data[i].value * 2.0f;
        
        /* Access dynamic array with complex indexing */
        int dyn_index = (i * thread_id) % (N * M);
        int dyn_val = dynamic_array[dyn_index];
        
        /* Nested loop for worker-level partitioning */
        #pragma omp simd
        for (j = 0; j < 4; j++) {
            /* Struct array member access - vector partitioning opportunities */
            int meta = data[i].metadata[j];
            partial_sums[team_id % M] += meta * dyn_val + (int)scaled;
        }
        
        /* Conditional update based on thread/team IDs */
        if (thread_id % 3 == 0) {
            data[i].value = scaled / (team_id + 1);
        }
    }
    
    /* Verify results */
    int total_sum = 0;
    for (i = 0; i < M; i++) {
        total_sum += partial_sums[i];
    }
    printf("OpenMP total sum: %d\n", total_sum);
    
    free(dynamic_array);
}

/* Hybrid test with both OpenACC and OpenMP for maximum coverage */
void test_hybrid_partitioning() {
    int i, j, k;
    int matrixA[N][N];
    int matrixB[N][N];
    int result[N][N];
    
    /* Initialize matrices */
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            matrixA[i][j] = i + j;
            matrixB[i][j] = i - j;
            result[i][j] = 0;
        }
    }
    
    /* Complex OpenACC kernel with multiple nesting levels */
    #pragma acc parallel loop gang collapse(2) \
        copyin(matrixA[0:N][0:N], matrixB[0:N][0:N]) \
        copy(result[0:N][0:N])
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            int sum = 0;
            
            /* Worker-level loop */
            #pragma acc loop worker reduction(+:sum)
            for (k = 0; k < N; k++) {
                /* Vector-level computation */
                #pragma acc loop vector
                for (int v = 0; v < 4; v++) {
                    sum += matrixA[i][k] * matrixB[k][j] + v;
                }
            }
            
            result[i][j] = sum;
            
            /* Additional conditional computation */
            if ((i + j) % 3 == 0) {
                #pragma acc loop seq
                for (int extra = 0; extra < 8; extra++) {
                    result[i][j] += extra * (i % 4);
                }
            }
        }
    }
    
    /* Verify with simple checksum */
    long checksum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            checksum += result[i][j];
        }
    }
    printf("Hybrid checksum: %ld\n", checksum);
}

int main() {
    printf("Testing GCC neuter-broadcast pass partitioning states...\n");
    
    /* Test 1: OpenACC with various data patterns */
    test_openacc_partitioning();
    
    /* Test 2: OpenMP with structs and dynamic data */
    test_openmp_partitioning();
    
    /* Test 3: Hybrid complex computation */
    test_hybrid_partitioning();
    
    printf("All tests completed.\n");
    return 0;
}
