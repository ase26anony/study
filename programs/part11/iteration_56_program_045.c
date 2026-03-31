/* Test program to cover partition mapping strings in GCC's OpenACC offload infrastructure.
   This program uses various OpenACC constructs to trigger different partition types
   (gang, worker, vector) and their combinations, aiming to exercise the internal
   partition code to string mapping function in omp-oacc-neuter-broadcast.cc. */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define GANGS 8
#define WORKERS 4
#define VECTORS 2
#define SIZE 1024

/* Global arrays to store results and prevent optimization */
int global_result[8] = {0};
volatile int volatile_sum = 0;

/* Test 1: Gang redundant partition (likely case 0) */
void test_gang_redundant(void) {
    int arr[SIZE];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;
    }
    
    /* Simple parallel reduction - likely gang redundant */
    #pragma acc parallel loop copyin(arr[0:SIZE]) copy(sum) reduction(+:sum)
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];
    }
    
    global_result[0] = sum;
    volatile_sum += sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int arr[GANGS][WORKERS];
    int partial_sums[GANGS] = {0};
    
    /* Initialize 2D array */
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            arr[g][w] = g * 10 + w;
        }
    }
    
    /* Each gang processes its own chunk - gang partitioned */
    #pragma acc parallel copyin(arr[0:GANGS][0:WORKERS]) copyout(partial_sums[0:GANGS]) \
        num_gangs(GANGS) num_workers(1) vector_length(1)
    {
        int gang_id = 0;
        #pragma acc loop gang private(gang_id)
        for (int g = 0; g < GANGS; g++) {
            gang_id = g;
            int local_sum = 0;
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                local_sum += arr[gang_id][w];
            }
            partial_sums[gang_id] = local_sum;
        }
    }
    
    /* Combine partial sums */
    int total = 0;
    for (int g = 0; g < GANGS; g++) {
        total += partial_sums[g];
    }
    global_result[1] = total;
    volatile_sum += total;
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[WORKERS][VECTORS];
    int worker_results[WORKERS] = {0};
    
    /* Initialize array */
    for (int w = 0; w < WORKERS; w++) {
        for (int v = 0; v < VECTORS; v++) {
            arr[w][v] = w * 5 + v;
        }
    }
    
    /* Explicit worker partitioning */
    #pragma acc parallel copyin(arr[0:WORKERS][0:VECTORS]) copyout(worker_results[0:WORKERS]) \
        num_gangs(1) num_workers(WORKERS) vector_length(1)
    {
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            int sum = 0;
            #pragma acc loop vector
            for (int v = 0; v < VECTORS; v++) {
                sum += arr[w][v];
            }
            worker_results[w] = sum;
        }
    }
    
    int total = 0;
    for (int w = 0; w < WORKERS; w++) {
        total += worker_results[w];
    }
    global_result[2] = total;
    volatile_sum += total;
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[GANGS][WORKERS][VECTORS];
    int results[GANGS][WORKERS];
    
    /* Initialize 3D array */
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                arr[g][w][v] = g * 100 + w * 10 + v;
            }
        }
    }
    
    /* Combined gang and worker partitioning */
    #pragma acc parallel copyin(arr[0:GANGS][0:WORKERS][0:VECTORS]) \
        copyout(results[0:GANGS][0:WORKERS]) \
        num_gangs(GANGS) num_workers(WORKERS) vector_length(1)
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                int sum = 0;
                #pragma acc loop vector
                for (int v = 0; v < VECTORS; v++) {
                    sum += arr[g][w][v];
                }
                results[g][w] = sum;
            }
        }
    }
    
    int total = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            total += results[g][w];
        }
    }
    global_result[3] = total;
    volatile_sum += total;
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[SIZE];
    int result[SIZE];
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Explicit vector partitioning */
    #pragma acc parallel copyin(arr[0:SIZE]) copyout(result[0:SIZE]) \
        num_gangs(1) num_workers(1) vector_length(VECTORS)
    {
        #pragma acc loop vector
        for (int i = 0; i < SIZE; i++) {
            result[i] = arr[i] * 2;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += result[i];
    }
    global_result[4] = sum;
    volatile_sum += sum;
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[GANGS][VECTORS];
    int results[GANGS];
    
    /* Initialize array */
    for (int g = 0; g < GANGS; g++) {
        for (int v = 0; v < VECTORS; v++) {
            arr[g][v] = g * 10 + v;
        }
    }
    
    /* Combined gang and vector partitioning */
    #pragma acc parallel copyin(arr[0:GANGS][0:VECTORS]) copyout(results[0:GANGS]) \
        num_gangs(GANGS) num_workers(1) vector_length(VECTORS)
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            int sum = 0;
            #pragma acc loop vector
            for (int v = 0; v < VECTORS; v++) {
                sum += arr[g][v];
            }
            results[g] = sum;
        }
    }
    
    int total = 0;
    for (int g = 0; g < GANGS; g++) {
        total += results[g];
    }
    global_result[5] = total;
    volatile_sum += total;
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[WORKERS][VECTORS];
    int results[WORKERS];
    
    /* Initialize array */
    for (int w = 0; w < WORKERS; w++) {
        for (int v = 0; v < VECTORS; v++) {
            arr[w][v] = w * 10 + v;
        }
    }
    
    /* Combined worker and vector partitioning */
    #pragma acc parallel copyin(arr[0:WORKERS][0:VECTORS]) copyout(results[0:WORKERS]) \
        num_gangs(1) num_workers(WORKERS) vector_length(VECTORS)
    {
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            int sum = 0;
            #pragma acc loop vector
            for (int v = 0; v < VECTORS; v++) {
                sum += arr[w][v];
            }
            results[w] = sum;
        }
    }
    
    int total = 0;
    for (int w = 0; w < WORKERS; w++) {
        total += results[w];
    }
    global_result[6] = total;
    volatile_sum += total;
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int arr[GANGS][WORKERS][VECTORS];
    int results[GANGS][WORKERS][VECTORS];
    
    /* Initialize 3D array */
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                arr[g][w][v] = g * 100 + w * 10 + v;
            }
        }
    }
    
    /* Fully partitioned across all levels */
    #pragma acc parallel copyin(arr[0:GANGS][0:WORKERS][0:VECTORS]) \
        copyout(results[0:GANGS][0:WORKERS][0:VECTORS]) \
        num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTORS)
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector
                for (int v = 0; v < VECTORS; v++) {
                    results[g][w][v] = arr[g][w][v] * 3;
                }
            }
        }
    }
    
    int total = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                total += results[g][w][v];
            }
        }
    }
    global_result[7] = total;
    volatile_sum += total;
}

int main(void) {
    printf("Starting OpenACC partition coverage test...\n");
    
    /* Execute all test functions to trigger different partition types */
    test_gang_redundant();
    printf("Test 1 (gang redundant) completed: %d\n", global_result[0]);
    
    test_gang_partitioned();
    printf("Test 2 (gang partitioned) completed: %d\n", global_result[1]);
    
    test_worker_partitioned();
    printf("Test 3 (worker partitioned) completed: %d\n", global_result[2]);
    
    test_gang_worker_partitioned();
    printf("Test 4 (gang+worker partitioned) completed: %d\n", global_result[3]);
    
    test_vector_partitioned();
    printf("Test 5 (vector partitioned) completed: %d\n", global_result[4]);
    
    test_gang_vector_partitioned();
    printf("Test 6 (gang+vector partitioned) completed: %d\n", global_result[5]);
    
    test_worker_vector_partitioned();
    printf("Test 7 (worker+vector partitioned) completed: %d\n", global_result[6]);
    
    test_fully_partitioned();
    printf("Test 8 (fully partitioned) completed: %d\n", global_result[7]);
    
    /* Final checksum to ensure all computations were performed */
    int final_checksum = 0;
    for (int i = 0; i < 8; i++) {
        final_checksum += global_result[i];
    }
    
    printf("Final checksum: %d (volatile sum: %d)\n", final_checksum, volatile_sum);
    printf("All tests completed successfully.\n");
    
    return 0;
}
