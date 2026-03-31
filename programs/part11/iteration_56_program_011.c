/* Test program to cover partition mapping strings in GCC's OpenACC offload infrastructure.
   Designed to trigger all cases (0-7) of the partition code to string mapping function
   in omp-oacc-neuter-broadcast.cc lines 335-343. */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define GANGS 8
#define WORKERS 4
#define VECTORS 2
#define N 1024

/* Global arrays to prevent optimization removal */
volatile int global_counter = 0;
int results[8] = {0};  /* Store results from each test case */

/* Test 1: Gang redundant (likely case 0) */
void test_gang_redundant(void) {
    int arr[N];
    int sum = 0;
    
    #pragma acc parallel loop gang reduction(+:sum) copyin(arr[0:N]) copy(sum)
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    
    results[0] = sum;
    global_counter++;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int arr[GANGS][N/GANGS];
    int partial_sums[GANGS] = {0};
    
    #pragma acc parallel loop gang copyin(arr[0:GANGS][0:N/GANGS]) copyout(partial_sums[0:GANGS])
    for (int g = 0; g < GANGS; g++) {
        int local_sum = 0;
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < N/GANGS; i++) {
            local_sum += arr[g][i];
        }
        partial_sums[g] = local_sum;
    }
    
    int total = 0;
    for (int g = 0; g < GANGS; g++) {
        total += partial_sums[g];
    }
    results[1] = total;
    global_counter++;
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[WORKERS][N/WORKERS];
    
    #pragma acc parallel loop worker copy(arr[0:WORKERS][0:N/WORKERS])
    for (int w = 0; w < WORKERS; w++) {
        for (int i = 0; i < N/WORKERS; i++) {
            arr[w][i] += 1;  /* Simple observable modification */
        }
    }
    
    /* Verify some changes */
    int check = 0;
    for (int w = 0; w < WORKERS; w++) {
        check += arr[w][0];
    }
    results[2] = check;
    global_counter++;
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[GANGS][WORKERS][N/(GANGS*WORKERS)];
    
    #pragma acc parallel loop gang worker copy(arr[0:GANGS][0:WORKERS][0:N/(GANGS*WORKERS)])
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int i = 0; i < N/(GANGS*WORKERS); i++) {
                arr[g][w][i] = g * 100 + w * 10 + i;
            }
        }
    }
    
    results[3] = arr[0][0][0];
    global_counter++;
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[N];
    
    #pragma acc parallel loop vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 2;
    }
    
    results[4] = arr[N/2];
    global_counter++;
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[GANGS][VECTORS][N/(GANGS*VECTORS)];
    
    #pragma acc parallel loop gang vector copy(arr[0:GANGS][0:VECTORS][0:N/(GANGS*VECTORS)])
    for (int g = 0; g < GANGS; g++) {
        for (int v = 0; v < VECTORS; v++) {
            for (int i = 0; i < N/(GANGS*VECTORS); i++) {
                arr[g][v][i] = (g << 16) | (v << 8) | i;
            }
        }
    }
    
    results[5] = arr[GANGS-1][VECTORS-1][0];
    global_counter++;
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[WORKERS][VECTORS][N/(WORKERS*VECTORS)];
    
    #pragma acc parallel loop worker vector copy(arr[0:WORKERS][0:VECTORS][0:N/(WORKERS*VECTORS)])
    for (int w = 0; w < WORKERS; w++) {
        for (int v = 0; v < VECTORS; v++) {
            for (int i = 0; i < N/(WORKERS*VECTORS); i++) {
                arr[w][v][i] = w * v * i;
            }
        }
    }
    
    results[6] = arr[WORKERS/2][VECTORS/2][0];
    global_counter++;
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int arr[GANGS][WORKERS][VECTORS];
    
    #pragma acc parallel loop gang worker vector collapse(3) copy(arr[0:GANGS][0:WORKERS][0:VECTORS])
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                arr[g][w][v] = g * WORKERS * VECTORS + w * VECTORS + v;
            }
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    #pragma acc parallel loop gang worker vector reduction(+:checksum) copyin(arr[0:GANGS][0:WORKERS][0:VECTORS]) copy(checksum)
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                checksum += arr[g][w][v];
            }
        }
    }
    
    results[7] = checksum;
    global_counter++;
}

/* Main driver that runs all test cases */
int main(void) {
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Initialize some data */
    srand(42);
    
    /* Run all test cases to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Verify all tests executed */
    printf("Tests completed. Global counter: %d\n", global_counter);
    
    /* Compute final checksum */
    int final_checksum = 0;
    for (int i = 0; i < 8; i++) {
        final_checksum += results[i];
    }
    printf("Final checksum: %d\n", final_checksum);
    
    /* Ensure all tests ran */
    assert(global_counter == 8);
    
    return 0;
}
