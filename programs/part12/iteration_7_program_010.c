/* test_openacc_partitions.c - Cover all partition mapping cases in omp-oacc-neuter-broadcast.cc */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50
#define P 25

/* Routine with gang partitioning */
#pragma acc routine seq
void init_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = i;
    }
}

/* Routine with vector partitioning */
#pragma acc routine vector
void vector_scale(float *arr, int size, float factor) {
    #pragma acc loop vector
    for (int i = 0; i < size; i++) {
        arr[i] *= factor;
    }
}

/* Routine with worker partitioning */
#pragma acc routine worker
void worker_process(int *arr, int size, int offset) {
    #pragma acc loop worker
    for (int i = 0; i < size; i++) {
        arr[i] += offset;
    }
}

/* Test 1: Basic partition combinations */
void test_basic_partitions(int argc) {
    int arr1[N][M];
    float arr2[N][M];
    
    /* Case 0: gang redundant */
    #pragma acc parallel copy(arr1) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr1[i][j] = i * M + j;
            }
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc kernels create(arr2) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2[i][j] = (float)(i + j);
            }
        }
    }
    
    /* Case 2: worker partitioned */
    if (argc > 1) {
        #pragma acc parallel copy(arr1) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr1[i][j] += 1;
                }
            }
        }
    }
}

/* Test 2: Multi-dimensional arrays with collapse */
void test_multi_dim_partitions() {
    int arr3d[N][M][P];
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel loop collapse(2) gang worker copy(arr3d)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            #pragma acc loop vector
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    /* Case 4: vector partitioned */
    #pragma acc kernels loop vector copy(arr3d)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] *= 2;
            }
        }
    }
}

/* Test 3: Nested and sequential regions */
void test_nested_regions(int argc) {
    float arr4[N][M];
    int arr5[N];
    
    /* Enter data with gang partitioning */
    #pragma acc enter data copyin(arr4) gang
    
    /* Outer region with gang partitioning */
    #pragma acc parallel present(arr4) gang if(argc > 2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Case 5: gang+vector partitioned */
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr4[i][j] = i * 0.5f + j * 0.25f;
            }
        }
    }
    
    /* Separate region with worker partitioning */
    #pragma acc kernels copy(arr5) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr5[i] = i * 3;
        }
    }
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel loop gang worker vector copy(arr5)
    for (int i = 0; i < N; i++) {
        arr5[i] += arr5[i] / 2;
    }
    
    #pragma acc exit data copyout(arr4)
}

/* Test 4: Complex routine interactions */
void test_routine_interactions() {
    float vec_arr[N*M];
    int worker_arr[N];
    
    /* Initialize on host */
    init_array(worker_arr, N);
    
    /* Enter data with different partition types */
    #pragma acc enter data copyin(vec_arr[0:N*M]) gang
    #pragma acc enter data copyin(worker_arr[0:N]) worker
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc parallel present(vec_arr, worker_arr) gang worker vector
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Call worker-partitioned routine */
            worker_process(&worker_arr[i], 1, i);
            
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                vec_arr[idx] = (float)(worker_arr[i] + j);
            }
        }
    }
    
    /* Call vector-partitioned routine */
    #pragma acc parallel present(vec_arr) vector
    {
        vector_scale(vec_arr, N*M, 1.5f);
    }
    
    #pragma acc exit data copyout(vec_arr, worker_arr)
}

/* Test 5: Mixed partition specifications */
void test_mixed_partitions(int argc) {
    int arr6[N][M][P];
    int arr7[N];
    
    /* Multiple clauses with different partitions */
    #pragma acc data copy(arr6) copy(arr7) \
        gang worker vector  /* Fully partitioned data region */
    {
        /* Region 1: gang partitioned loop */
        #pragma acc parallel loop gang collapse(2) present(arr6)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr6[i][j][k] = 1;
                }
            }
        }
        
        /* Region 2: worker partitioned with if condition */
        if (argc > 3) {
            #pragma acc kernels loop worker present(arr7)
            for (int i = 0; i < N; i++) {
                arr7[i] = i * i;
            }
        }
        
        /* Region 3: vector partitioned update */
        #pragma acc parallel loop vector present(arr6, arr7)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr6[i][j][k] += arr7[i];
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all tests with conditional paths based on argc
       to prevent dead code elimination */
    test_basic_partitions(argc);
    test_multi_dim_partitions();
    test_nested_regions(argc);
    test_routine_interactions();
    test_mixed_partitions(argc);
    
    /* Final validation region with all partition types */
    int final_arr[10][20][30];
    
    #pragma acc parallel loop collapse(3) gang worker vector copy(final_arr)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                final_arr[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Verify results on host */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                sum += final_arr[i][j][k];
            }
        }
    }
    
    printf("Final sum: %d\n", sum);
    printf("Test completed.\n");
    
    return 0;
}
