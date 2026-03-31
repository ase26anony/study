/* Test program to cover partition mapping in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
int global_arr_3d[G][W][V];
int global_arr_2d[W][V];
int global_arr_1d[N];
volatile int volatile_sum = 0;

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    int arr[N];
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i % 100;
    }
    
    /* OpenACC parallel region - likely gang redundant for reduction */
    #pragma acc parallel loop copy(arr[0:N]) reduction(+:local_sum)
    for (int i = 0; i < N; i++) {
        local_sum += arr[i];
    }
    
    volatile_sum += local_sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int arr[G][W];
    
    /* Initialize */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            arr[i][j] = i * W + j;
        }
    }
    
    /* Each gang processes its own chunk */
    #pragma acc parallel copy(arr[0:G][0:W]) num_gangs(G)
    {
        #pragma acc loop gang
        for (int i = 0; i < G; i++) {
            #pragma acc loop worker
            for (int j = 0; j < W; j++) {
                arr[i][j] *= 2;
            }
        }
    }
    
    /* Store result to global */
    #pragma acc parallel copy(arr[0:G][0:W]) copyout(global_arr_2d[0:G][0:W])
    {
        #pragma acc loop gang
        for (int i = 0; i < G; i++) {
            #pragma acc loop worker
            for (int j = 0; j < W; j++) {
                global_arr_2d[i][j] = arr[i][j];
            }
        }
    }
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[W][V];
    
    /* Initialize */
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr[i][j] = i * V + j;
        }
    }
    
    /* Explicit worker partitioning */
    #pragma acc parallel copy(arr[0:W][0:V]) num_workers(W)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            #pragma acc loop vector
            for (int j = 0; j < V; j++) {
                arr[i][j] += 1;
            }
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[G][W];
    
    /* Initialize */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            arr[i][j] = i * W + j;
        }
    }
    
    /* Combined gang and worker partitioning */
    #pragma acc parallel copy(arr[0:G][0:W]) num_gangs(G) num_workers(W)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                arr[i][j] = arr[i][j] * 3 + 1;
            }
        }
    }
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    /* Explicit vector partitioning */
    #pragma acc parallel copy(arr[0:N]) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr[i] = arr[i] * 2;
        }
    }
    
    /* Copy to global */
    #pragma acc parallel copy(arr[0:N]) copyout(global_arr_1d[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            global_arr_1d[i] = arr[i];
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[G][V];
    
    /* Initialize */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < V; j++) {
            arr[i][j] = i * V + j;
        }
    }
    
    /* Combined gang and vector partitioning */
    #pragma acc parallel copy(arr[0:G][0:V]) num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < V; j++) {
                arr[i][j] += 5;
            }
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[W][V];
    
    /* Initialize */
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr[i][j] = i * V + j;
        }
    }
    
    /* Combined worker and vector partitioning */
    #pragma acc parallel copy(arr[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                arr[i][j] *= 2;
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int arr[G][W][V];
    
    /* Initialize 3D array */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr[i][j][k] = i * W * V + j * V + k;
            }
        }
    }
    
    /* Fully partitioned across all levels */
    #pragma acc parallel copy(arr[0:G][0:W][0:V]) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    arr[i][j][k] = arr[i][j][k] + 1;
                }
            }
        }
    }
    
    /* Store to global */
    #pragma acc parallel copy(arr[0:G][0:W][0:V]) \
                copyout(global_arr_3d[0:G][0:W][0:V])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    global_arr_3d[i][j][k] = arr[i][j][k];
                }
            }
        }
    }
}

/* Main function to run all tests */
int main(void) {
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Run all test functions to trigger different partition mappings */
    test_gang_redundant();
    printf("Test 1 (gang redundant) completed\n");
    
    test_gang_partitioned();
    printf("Test 2 (gang partitioned) completed\n");
    
    test_worker_partitioned();
    printf("Test 3 (worker partitioned) completed\n");
    
    test_gang_worker_partitioned();
    printf("Test 4 (gang+worker partitioned) completed\n");
    
    test_vector_partitioned();
    printf("Test 5 (vector partitioned) completed\n");
    
    test_gang_vector_partitioned();
    printf("Test 6 (gang+vector partitioned) completed\n");
    
    test_worker_vector_partitioned();
    printf("Test 7 (worker+vector partitioned) completed\n");
    
    test_fully_partitioned();
    printf("Test 8 (fully partitioned) completed\n");
    
    /* Compute checksum to ensure all computations happened */
    int checksum = volatile_sum;
    
    #pragma acc parallel loop reduction(+:checksum) copyin(global_arr_1d[0:N])
    for (int i = 0; i < N; i++) {
        checksum += global_arr_1d[i];
    }
    
    #pragma acc parallel loop reduction(+:checksum) copyin(global_arr_2d[0:G][0:W])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            checksum += global_arr_2d[i][j];
        }
    }
    
    #pragma acc parallel loop reduction(+:checksum) copyin(global_arr_3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                checksum += global_arr_3d[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed successfully!\n");
    
    return 0;
}
