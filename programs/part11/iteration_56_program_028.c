/* test_openacc_partitions.c
 * Designed to exercise GCC's OpenACC partition mapping logic
 * to cover lines 335-343 in omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define GANGS 4
#define WORKERS 2
#define VECTORS 32
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int results[8] = {0};

/* Test 1: Gang redundant partitioning */
void test_gang_redundant(void) {
    int arr[N];
    int sum = 0;
    
    #pragma acc parallel copy(arr[0:N]) copyout(sum) num_gangs(GANGS)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            arr[i] = i;
            sum += arr[i];
        }
    }
    
    results[0] = sum;
    global_sum += sum;
}

/* Test 2: Gang partitioned */
void test_gang_partitioned(void) {
    int arr[GANGS][N/GANGS];
    int partial_sums[GANGS] = {0};
    
    #pragma acc parallel copy(arr) copyout(partial_sums) num_gangs(GANGS)
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            int local_sum = 0;
            #pragma acc loop vector
            for (int i = 0; i < N/GANGS; i++) {
                arr[g][i] = g * (N/GANGS) + i;
                local_sum += arr[g][i];
            }
            partial_sums[g] = local_sum;
        }
    }
    
    int total = 0;
    for (int g = 0; g < GANGS; g++) {
        total += partial_sums[g];
    }
    results[1] = total;
    global_sum += total;
}

/* Test 3: Worker partitioned */
void test_worker_partitioned(void) {
    int arr[WORKERS][N/WORKERS];
    
    #pragma acc parallel copy(arr) num_gangs(1) num_workers(WORKERS)
    {
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            #pragma acc loop vector
            for (int i = 0; i < N/WORKERS; i++) {
                arr[w][i] = w * (N/WORKERS) + i;
            }
        }
    }
    
    int sum = 0;
    for (int w = 0; w < WORKERS; w++) {
        for (int i = 0; i < N/WORKERS; i++) {
            sum += arr[w][i];
        }
    }
    results[2] = sum;
    global_sum += sum;
}

/* Test 4: Gang+worker partitioned */
void test_gang_worker_partitioned(void) {
    int arr[GANGS][WORKERS][N/(GANGS*WORKERS)];
    
    #pragma acc parallel copy(arr) num_gangs(GANGS) num_workers(WORKERS)
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector
                for (int i = 0; i < N/(GANGS*WORKERS); i++) {
                    arr[g][w][i] = g * WORKERS * (N/(GANGS*WORKERS)) + 
                                   w * (N/(GANGS*WORKERS)) + i;
                }
            }
        }
    }
    
    int sum = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int i = 0; i < N/(GANGS*WORKERS); i++) {
                sum += arr[g][w][i];
            }
        }
    }
    results[3] = sum;
    global_sum += sum;
}

/* Test 5: Vector partitioned */
void test_vector_partitioned(void) {
    int arr[N];
    
    #pragma acc parallel copy(arr) vector_length(VECTORS)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr[i] = i * 2;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    results[4] = sum;
    global_sum += sum;
}

/* Test 6: Gang+vector partitioned */
void test_gang_vector_partitioned(void) {
    int arr[GANGS][N/GANGS];
    
    #pragma acc parallel copy(arr) num_gangs(GANGS) vector_length(VECTORS)
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop vector
            for (int i = 0; i < N/GANGS; i++) {
                arr[g][i] = g * (N/GANGS) + i;
            }
        }
    }
    
    int sum = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int i = 0; i < N/GANGS; i++) {
            sum += arr[g][i];
        }
    }
    results[5] = sum;
    global_sum += sum;
}

/* Test 7: Worker+vector partitioned */
void test_worker_vector_partitioned(void) {
    int arr[WORKERS][N/WORKERS];
    
    #pragma acc parallel copy(arr) num_workers(WORKERS) vector_length(VECTORS)
    {
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            #pragma acc loop vector
            for (int i = 0; i < N/WORKERS; i++) {
                arr[w][i] = w * (N/WORKERS) + i;
            }
        }
    }
    
    int sum = 0;
    for (int w = 0; w < WORKERS; w++) {
        for (int i = 0; i < N/WORKERS; i++) {
            sum += arr[w][i];
        }
    }
    results[6] = sum;
    global_sum += sum;
}

/* Test 8: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(void) {
    int arr[GANGS][WORKERS][VECTORS];
    
    #pragma acc parallel copy(arr) num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTORS)
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector
                for (int v = 0; v < VECTORS; v++) {
                    arr[g][w][v] = g * WORKERS * VECTORS + w * VECTORS + v;
                }
            }
        }
    }
    
    int sum = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                sum += arr[g][w][v];
            }
        }
    }
    results[7] = sum;
    global_sum += sum;
}

int main(void) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all test cases to trigger different partition types */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Verify results and prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < 8; i++) {
        final_sum += results[i];
    }
    
    printf("Global sum: %d\n", global_sum);
    printf("Final checksum: %d\n", final_sum);
    
    /* Use results to prevent optimization */
    if (final_sum != global_sum) {
        printf("ERROR: Checksum mismatch!\n");
        return 1;
    }
    
    return 0;
}
