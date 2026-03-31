/* Test program to cover partition mapping strings in GCC's OpenACC neuter/broadcast pass */
#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int arr1d[N];
int arr2d[W][N/W];
int arr3d[G][W][V];

/* Test 1: Gang redundant partition (case 0) */
void test_gang_redundant(void) {
    int sum = 0;
    
    #pragma acc parallel loop gang reduction(+:sum) copyin(arr1d[0:N]) copy(sum)
    for (int i = 0; i < N; i++) {
        sum += arr1d[i];
    }
    
    global_sum += sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    #pragma acc parallel loop gang copy(arr1d[0:N])
    for (int i = 0; i < N; i++) {
        arr1d[i] += 1;
    }
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel loop worker copy(arr2d[0:W][0:N/W])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < N/W; j++) {
            arr2d[i][j] = i * 100 + j;
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel loop gang worker collapse(2) copy(arr3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = i * 1000 + j * 100 + k;
            }
        }
    }
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel loop vector copy(arr1d[0:N])
    for (int i = 0; i < N; i++) {
        arr1d[i] *= 2;
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel loop gang vector copy(arr1d[0:N])
    for (int i = 0; i < N; i++) {
        arr1d[i] += i;
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel loop worker vector copy(arr2d[0:W][0:N/W])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < N/W; j++) {
            arr2d[i][j] += arr2d[i][j] / 2;
        }
    }
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    #pragma acc parallel loop gang worker vector collapse(3) copy(arr3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = arr3d[i][j][k] * 3 + 1;
            }
        }
    }
}

/* Test with explicit num_gangs, num_workers, vector_length */
void test_explicit_partitioning(void) {
    int local_arr[G][W][V];
    
    #pragma acc parallel copy(local_arr[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang
        for (int i = 0; i < G; i++) {
            #pragma acc loop worker
            for (int j = 0; j < W; j++) {
                #pragma acc loop vector
                for (int k = 0; k < V; k++) {
                    local_arr[i][j][k] = i + j + k;
                }
            }
        }
    }
    
    /* Use the result to prevent optimization */
    int sum = 0;
    #pragma acc parallel loop reduction(+:sum) copyin(local_arr[0:G][0:W][0:V]) copy(sum)
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                sum += local_arr[i][j][k];
            }
        }
    }
    global_sum += sum;
}

/* Test with data regions and update directives */
void test_data_regions(void) {
    int data[G][W][V];
    
    #pragma acc data copy(data[0:G][0:W][0:V])
    {
        #pragma acc parallel loop gang worker vector collapse(3)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    data[i][j][k] = 1;
                }
            }
        }
        
        #pragma acc update host(data[0:G][0:W][0:V])
    }
    
    /* Verify data */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                if (data[i][j][k] != 1) {
                    printf("Data verification failed!\n");
                }
            }
        }
    }
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i % 100;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < N/W; j++) {
            arr2d[i][j] = 0;
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = 0;
            }
        }
    }
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Execute all test functions to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_explicit_partitioning();
    test_data_regions();
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += arr1d[i];
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < N/W; j++) {
            final_sum += arr2d[i][j];
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                final_sum += arr3d[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %d\n", final_sum + global_sum);
    printf("Tests completed.\n");
    
    return 0;
}
