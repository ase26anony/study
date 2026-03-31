/* test_openacc_partitions.c
 * Designed to exercise GCC's internal partition mapping function
 * for OpenACC data partitioning across gang, worker, and vector levels.
 */

#include <stdio.h>
#include <stdlib.h>

#define G 8   /* gangs */
#define W 4   /* workers */
#define V 32  /* vector length */
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int results[8] = {0};  /* Store results from different test cases */

/* Test 1: Gang redundant partitioning (likely case 0) */
void test_gang_redundant(void) {
    int arr[N];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i % 100;
    }
    
    #pragma acc parallel copy(arr[0:N]) copy(sum) num_gangs(G)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
    }
    
    results[0] = sum;
    global_sum += sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int arr[G][N/G];
    int partial_sums[G] = {0};
    
    /* Initialize 2D array */
    for (int g = 0; g < G; g++) {
        for (int i = 0; i < N/G; i++) {
            arr[g][i] = (g * 100 + i) % 1000;
        }
    }
    
    #pragma acc parallel copy(arr[0:G][0:N/G]) copyout(partial_sums[0:G]) \
        num_gangs(G) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            int local_sum = 0;
            #pragma acc loop seq
            for (int i = 0; i < N/G; i++) {
                local_sum += arr[g][i];
            }
            partial_sums[g] = local_sum;
        }
    }
    
    int total = 0;
    for (int g = 0; g < G; g++) {
        total += partial_sums[g];
    }
    results[1] = total;
    global_sum += total;
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[W][N/W];
    
    /* Initialize array */
    for (int w = 0; w < W; w++) {
        for (int i = 0; i < N/W; i++) {
            arr[w][i] = w * 10 + i;
        }
    }
    
    #pragma acc parallel copy(arr[0:W][0:N/W]) num_gangs(1) num_workers(W) vector_length(1)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < W; w++) {
            #pragma acc loop seq
            for (int i = 0; i < N/W; i++) {
                arr[w][i] *= 2;  /* Simple transformation */
            }
        }
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int w = 0; w < W; w++) {
        for (int i = 0; i < N/W; i++) {
            sum += arr[w][i];
        }
    }
    results[2] = sum;
    global_sum += sum;
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[G][W][N/(G*W)];
    
    /* Initialize 3D array */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int i = 0; i < N/(G*W); i++) {
                arr[g][w][i] = g * 1000 + w * 100 + i;
            }
        }
    }
    
    #pragma acc parallel copy(arr[0:G][0:W][0:N/(G*W)]) \
        num_gangs(G) num_workers(W) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < W; w++) {
                #pragma acc loop seq
                for (int i = 0; i < N/(G*W); i++) {
                    arr[g][w][i] += 1;
                }
            }
        }
    }
    
    int sum = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int i = 0; i < N/(G*W); i++) {
                sum += arr[g][w][i];
            }
        }
    }
    results[3] = sum;
    global_sum += sum;
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    #pragma acc parallel copy(arr[0:N]) num_gangs(1) num_workers(1) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr[i] = arr[i] * 3 + 1;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    results[4] = sum;
    global_sum += sum;
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[G][N/G];
    
    for (int g = 0; g < G; g++) {
        for (int i = 0; i < N/G; i++) {
            arr[g][i] = g * 100 + i;
        }
    }
    
    #pragma acc parallel copy(arr[0:G][0:N/G]) num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop vector
            for (int i = 0; i < N/G; i++) {
                arr[g][i] = arr[g][i] * 2 - 1;
            }
        }
    }
    
    int sum = 0;
    for (int g = 0; g < G; g++) {
        for (int i = 0; i < N/G; i++) {
            sum += arr[g][i];
        }
    }
    results[5] = sum;
    global_sum += sum;
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[W][N/W];
    
    for (int w = 0; w < W; w++) {
        for (int i = 0; i < N/W; i++) {
            arr[w][i] = w * 50 + i;
        }
    }
    
    #pragma acc parallel copy(arr[0:W][0:N/W]) num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector
            for (int i = 0; i < N/W; i++) {
                arr[w][i] += w * 10;
            }
        }
    }
    
    int sum = 0;
    for (int w = 0; w < W; w++) {
        for (int i = 0; i < N/W; i++) {
            sum += arr[w][i];
        }
    }
    results[6] = sum;
    global_sum += sum;
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int arr[G][W][V];
    
    /* Initialize 3D array matching gang/worker/vector dimensions */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr[g][w][v] = g * 10000 + w * 1000 + v;
            }
        }
    }
    
    #pragma acc parallel copy(arr[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector
                for (int v = 0; v < V; v++) {
                    arr[g][w][v] = (arr[g][w][v] * 7) / 3;
                }
            }
        }
    }
    
    int sum = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                sum += arr[g][w][v];
            }
        }
    }
    results[7] = sum;
    global_sum += sum;
}

int main(void) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    printf("Test 1 (gang redundant) complete: %d\n", results[0]);
    
    test_gang_partitioned();
    printf("Test 2 (gang partitioned) complete: %d\n", results[1]);
    
    test_worker_partitioned();
    printf("Test 3 (worker partitioned) complete: %d\n", results[2]);
    
    test_gang_worker_partitioned();
    printf("Test 4 (gang+worker partitioned) complete: %d\n", results[3]);
    
    test_vector_partitioned();
    printf("Test 5 (vector partitioned) complete: %d\n", results[4]);
    
    test_gang_vector_partitioned();
    printf("Test 6 (gang+vector partitioned) complete: %d\n", results[5]);
    
    test_worker_vector_partitioned();
    printf("Test 7 (worker+vector partitioned) complete: %d\n", results[6]);
    
    test_fully_partitioned();
    printf("Test 8 (fully partitioned) complete: %d\n", results[7]);
    
    printf("Global checksum: %d\n", global_sum);
    printf("All tests completed.\n");
    
    return 0;
}
