/* test_omp_acc_partitions.c
 * Designed to trigger partition string mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test_omp_acc_partitions.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50
#define P 25

/* Routine with explicit partition specification */
#pragma acc routine seq
void init_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = i;
    }
}

/* Gang-partitioned routine */
#pragma acc routine gang
void gang_operation(int *arr, int size, int factor) {
    #pragma acc loop gang
    for (int i = 0; i < size; i++) {
        arr[i] *= factor;
    }
}

/* Vector-partitioned routine */
#pragma acc routine vector
void vector_operation(int *arr, int size, int offset) {
    #pragma acc loop vector
    for (int i = 0; i < size; i++) {
        arr[i] += offset;
    }
}

/* Test 1: Basic partition combinations */
void test_basic_partitions(int argc) {
    int arr1[N][M][P];
    int arr2[N][M];
    int arr3[N];
    
    /* Initialize arrays */
    #pragma acc parallel loop collapse(3) gang, worker, vector copy(arr1[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr1[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Gang redundant partitioning */
    if (argc > 1) {
        #pragma acc kernels copy(arr2[0:N][0:M]) gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2[i][j] = arr1[i][j][0] * 2;
            }
        }
    }
    
    /* Gang partitioned */
    #pragma acc parallel copy(arr3[0:N]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr3[i] = i * 3;
        }
    }
    
    /* Worker partitioned */
    #pragma acc kernels create(arr2[0:N][0:M]) worker
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            arr2[i][j] += arr3[i];
        }
    }
    
    /* Gang+worker partitioned */
    int temp[N][M];
    #pragma acc enter data copyin(temp[0:N][0:M]) gang, worker
    #pragma acc parallel present(temp) gang, worker
    {
        #pragma acc loop gang, worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                temp[i][j] = arr2[i][j] + arr3[i % N];
            }
        }
    }
    #pragma acc exit data copyout(temp[0:N][0:M])
}

/* Test 2: Vector partitioning combinations */
void test_vector_partitions(int use_vector) {
    int vec_arr[N][M];
    
    /* Vector partitioned */
    if (use_vector) {
        #pragma acc parallel loop vector copy(vec_arr[0:N][0:M])
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                vec_arr[i][j] = i * M + j;
            }
        }
    }
    
    /* Gang+vector partitioned */
    int gv_arr[N];
    #pragma acc kernels copy(gv_arr[0:N]) gang, vector
    for (int i = 0; i < N; i++) {
        gv_arr[i] = 0;
        #pragma acc loop gang, vector
        for (int iter = 0; iter < 10; iter++) {
            gv_arr[i] += vec_arr[i % N][iter % M];
        }
    }
    
    /* Worker+vector partitioned */
    int wv_arr[M];
    #pragma acc parallel copy(wv_arr[0:M]) worker, vector
    {
        #pragma acc loop worker, vector
        for (int j = 0; j < M; j++) {
            wv_arr[j] = 0;
            for (int i = 0; i < 5; i++) {
                wv_arr[j] += vec_arr[i][j];
            }
        }
    }
    
    /* Fully partitioned (gang+worker+vector) */
    int full_arr[N][M];
    #pragma acc enter data create(full_arr[0:N][0:M]) gang, worker, vector
    
    #pragma acc parallel present(full_arr) gang, worker, vector
    {
        #pragma acc loop gang, worker, vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                full_arr[i][j] = vec_arr[i][j] + gv_arr[i % N] + wv_arr[j % M];
            }
        }
    }
    
    #pragma acc exit data copyout(full_arr[0:N][0:M])
}

/* Test 3: Nested regions and conditional offloading */
void test_nested_regions(int condition) {
    int nested_arr[N][M][P];
    
    /* Outer region with gang partitioning */
    #pragma acc parallel if(condition) copy(nested_arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Inner region with worker partitioning */
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                /* Innermost with vector partitioning */
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    nested_arr[i][j][k] = (i * 1000) + (j * 100) + k;
                }
            }
        }
    }
    
    /* Sequential region with mixed partitioning */
    int seq_arr[N];
    #pragma acc kernels copy(seq_arr[0:N]) gang(static:4), vector(128)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            int sum = 0;
            #pragma acc loop vector reduction(+:sum)
            for (int j = 0; j < M; j++) {
                sum += nested_arr[i][j % P][0];
            }
            seq_arr[i] = sum;
        }
    }
}

/* Test 4: Routine calls with partitioning */
void test_routine_partitions() {
    int routine_arr[N * M];
    int size = N * M;
    
    /* Initialize using seq routine */
    #pragma acc parallel copy(routine_arr[0:size])
    {
        init_array(routine_arr, size);
    }
    
    /* Call gang-partitioned routine */
    #pragma acc parallel copy(routine_arr[0:size]) gang
    {
        gang_operation(routine_arr, size, 2);
    }
    
    /* Call vector-partitioned routine */
    #pragma acc parallel copy(routine_arr[0:size]) vector
    {
        vector_operation(routine_arr, size, 100);
    }
    
    /* Mixed routine calls with collapse */
    int result[N][M];
    #pragma acc parallel loop collapse(2) gang, worker, vector copy(result[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            result[i][j] = routine_arr[idx] + (i * j);
        }
    }
}

/* Test 5: Complex data environment with persistent partitions */
void test_persistent_partitions(int argc) {
    static int persist_arr[N][M];
    static int persist_arr2[N];
    
    /* Establish device data region with gang partitioning */
    #pragma acc enter data copyin(persist_arr[0:N][0:M]) gang
    
    /* Multiple compute regions accessing partitioned data */
    for (int iter = 0; iter < 3; iter++) {
        if (iter == 0) {
            #pragma acc parallel present(persist_arr) worker
            {
                #pragma acc loop worker
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        persist_arr[i][j] = i * M + j + iter;
                    }
                }
            }
        } else if (iter == 1 && argc > 2) {
            #pragma acc parallel present(persist_arr) vector
            {
                #pragma acc loop vector
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        persist_arr[i][j] *= 2;
                    }
                }
            }
        } else {
            #pragma acc parallel present(persist_arr) gang, worker, vector
            {
                #pragma acc loop gang, worker, vector collapse(2)
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        persist_arr[i][j] += 1;
                    }
                }
            }
        }
    }
    
    /* Second array with different partition */
    #pragma acc enter data create(persist_arr2[0:N]) vector
    
    #pragma acc parallel present(persist_arr, persist_arr2) gang, vector
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            int sum = 0;
            #pragma acc loop vector reduction(+:sum)
            for (int j = 0; j < M; j++) {
                sum += persist_arr[i][j];
            }
            persist_arr2[i] = sum;
        }
    }
    
    /* Exit data with partition specifications */
    #pragma acc exit data copyout(persist_arr[0:N][0:M]) gang
    #pragma acc exit data copyout(persist_arr2[0:N]) vector
}

int main(int argc, char *argv[]) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Use argc to create conditional execution paths */
    int use_vector = (argc > 0);
    int condition = (argc % 2 == 0);
    
    /* Execute all test functions to cover different partition combinations */
    test_basic_partitions(argc);
    test_vector_partitions(use_vector);
    test_nested_regions(condition);
    test_routine_partitions();
    test_persistent_partitions(argc);
    
    /* Final validation on host */
    int final_check = 0;
    int check_arr[10];
    
    #pragma acc parallel loop gang, worker, vector copy(check_arr[0:10])
    for (int i = 0; i < 10; i++) {
        check_arr[i] = i * i;
        final_check += i;
    }
    
    printf("Test completed. Final check: %d\n", final_check);
    
    /* Verify results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += check_arr[i];
    }
    
    if (sum != 285) {  /* 0+1+4+9+16+25+36+49+64+81 = 285 */
        printf("Validation error: sum = %d\n", sum);
        return 1;
    }
    
    return 0;
}
