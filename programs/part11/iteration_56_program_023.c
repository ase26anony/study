/* Test program to cover partition mapping strings in GCC's OpenACC neuter/broadcast pass */
#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int arr_1d[N];
int arr_2d[W][V];
int arr_3d[G][W][V];
int results[8] = {0}; /* Store results from different tests */

/* Test 1: Gang redundant (likely case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel loop gang reduction(+:local_sum) copyin(arr_1d[0:N]) copy(local_sum)
    for (int i = 0; i < N; i++) {
        local_sum += arr_1d[i];
    }
    
    results[0] = local_sum;
    global_sum += local_sum;
}

/* Test 2: Gang partitioned (likely case 1) */
void test_gang_partitioned(void) {
    #pragma acc parallel loop gang copy(arr_1d[0:N])
    for (int i = 0; i < N; i++) {
        arr_1d[i] = arr_1d[i] * 2 + 1;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr_1d[i];
    }
    results[1] = sum;
}

/* Test 3: Worker partitioned (likely case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel loop worker copy(arr_2d[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr_2d[i][j] = i * V + j + 1;
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop worker reduction(+:sum) copyin(arr_2d[0:W][0:V]) copy(sum)
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            sum += arr_2d[i][j];
        }
    }
    results[2] = sum;
}

/* Test 4: Gang+worker partitioned (likely case 3) */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel loop gang worker copy(arr_3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr_3d[i][j][k] = (i * W * V) + (j * V) + k + 1;
            }
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop gang worker reduction(+:sum) copyin(arr_3d[0:G][0:W][0:V]) copy(sum)
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                sum += arr_3d[i][j][k];
            }
        }
    }
    results[3] = sum;
}

/* Test 5: Vector partitioned (likely case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel loop vector copy(arr_1d[0:N])
    for (int i = 0; i < N; i++) {
        arr_1d[i] = arr_1d[i] + i;
    }
    
    int sum = 0;
    #pragma acc parallel loop vector reduction(+:sum) copyin(arr_1d[0:N]) copy(sum)
    for (int i = 0; i < N; i++) {
        sum += arr_1d[i];
    }
    results[4] = sum;
}

/* Test 6: Gang+vector partitioned (likely case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel loop gang vector copy(arr_2d[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr_2d[i][j] = arr_2d[i][j] * 3 - 2;
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop gang vector reduction(+:sum) copyin(arr_2d[0:W][0:V]) copy(sum)
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            sum += arr_2d[i][j];
        }
    }
    results[5] = sum;
}

/* Test 7: Worker+vector partitioned (likely case 6) */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel loop worker vector copy(arr_2d[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr_2d[i][j] = (arr_2d[i][j] + i + j) % 100;
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop worker vector reduction(+:sum) copyin(arr_2d[0:W][0:V]) copy(sum)
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            sum += arr_2d[i][j];
        }
    }
    results[6] = sum;
}

/* Test 8: Fully partitioned (likely case 7) */
void test_fully_partitioned(void) {
    #pragma acc parallel loop gang worker vector collapse(3) copy(arr_3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr_3d[i][j][k] = arr_3d[i][j][k] * 2 - (i + j + k);
            }
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop gang worker vector collapse(3) reduction(+:sum) \
                copyin(arr_3d[0:G][0:W][0:V]) copy(sum)
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                sum += arr_3d[i][j][k];
            }
        }
    }
    results[7] = sum;
}

/* Initialize arrays with non-zero values */
void init_arrays(void) {
    for (int i = 0; i < N; i++) {
        arr_1d[i] = (i % 10) + 1;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr_2d[i][j] = (i * V + j) % 7 + 1;
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr_3d[i][j][k] = ((i * W * V) + (j * V) + k) % 13 + 1;
            }
        }
    }
}

int main(void) {
    /* Initialize test data */
    init_arrays();
    
    printf("Running OpenACC partition coverage tests...\n");
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Compute final checksum */
    int final_sum = 0;
    for (int i = 0; i < 8; i++) {
        final_sum += results[i];
    }
    final_sum += global_sum;
    
    printf("Test results checksum: %d\n", final_sum);
    printf("All partition tests completed.\n");
    
    return 0;
}
