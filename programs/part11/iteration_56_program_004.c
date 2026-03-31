/* test_openacc_partitions.c
 * Designed to trigger all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_openacc_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int results[8] = {0};

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant(void) {
    int arr[N];
    int sum = 0;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(arr[0:N]) copy(sum)
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        sum += arr[i];
    }
    
    results[0] = sum;
    global_sum += sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int arr[G][W];
    
    #pragma acc parallel loop gang copy(arr[0:G][0:W])
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            arr[g][w] = g * 100 + w;
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop gang reduction(+:sum) copy(arr[0:G][0:W]) copy(sum)
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            sum += arr[g][w];
        }
    }
    
    results[1] = sum;
    global_sum += sum;
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[W][V];
    
    #pragma acc parallel loop worker copy(arr[0:W][0:V])
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr[w][v] = w * 10 + v;
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop worker reduction(+:sum) copy(arr[0:W][0:V]) copy(sum)
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            sum += arr[w][v];
        }
    }
    
    results[2] = sum;
    global_sum += sum;
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[G][W][V];
    
    #pragma acc parallel loop gang worker copy(arr[0:G][0:W][0:V])
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr[g][w][v] = g * 1000 + w * 100 + v;
            }
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop gang worker reduction(+:sum) copy(arr[0:G][0:W][0:V]) copy(sum)
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                sum += arr[g][w][v];
            }
        }
    }
    
    results[3] = sum;
    global_sum += sum;
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[N];
    
    #pragma acc parallel loop vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 2;
    }
    
    int sum = 0;
    #pragma acc parallel loop vector reduction(+:sum) copy(arr[0:N]) copy(sum)
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    
    results[4] = sum;
    global_sum += sum;
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[G][V];
    
    #pragma acc parallel loop gang vector copy(arr[0:G][0:V])
    for (int g = 0; g < G; g++) {
        for (int v = 0; v < V; v++) {
            arr[g][v] = g * 100 + v;
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop gang vector reduction(+:sum) copy(arr[0:G][0:V]) copy(sum)
    for (int g = 0; g < G; g++) {
        for (int v = 0; v < V; v++) {
            sum += arr[g][v];
        }
    }
    
    results[5] = sum;
    global_sum += sum;
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[W][V];
    
    #pragma acc parallel loop worker vector copy(arr[0:W][0:V])
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr[w][v] = w * 10 + v;
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop worker vector reduction(+:sum) copy(arr[0:W][0:V]) copy(sum)
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            sum += arr[w][v];
        }
    }
    
    results[6] = sum;
    global_sum += sum;
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int arr[G][W][V];
    
    #pragma acc parallel loop gang worker vector collapse(3) copy(arr[0:G][0:W][0:V])
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr[g][w][v] = g * 1000 + w * 100 + v * 10;
            }
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop gang worker vector collapse(3) reduction(+:sum) copy(arr[0:G][0:W][0:V]) copy(sum)
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

/* Test 9: Mixed partition types in single region */
void test_mixed_partitions(void) {
    int arr1[N], arr2[N], arr3[N];
    
    /* Multiple data clauses with different array shapes */
    #pragma acc parallel copy(arr1[0:N]) copy(arr2[0:N/2]) copy(arr3[0:N/4]) \
        num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1[i] = i;
        }
        
        #pragma acc loop worker
        for (int i = 0; i < N/2; i++) {
            arr2[i] = i * 2;
        }
        
        #pragma acc loop vector
        for (int i = 0; i < N/4; i++) {
            arr3[i] = i * 3;
        }
    }
    
    int sum = 0;
    #pragma acc parallel loop reduction(+:sum) copy(arr1[0:N], arr2[0:N/2], arr3[0:N/4]) copy(sum)
    for (int i = 0; i < N; i++) {
        if (i < N/2) sum += arr2[i];
        if (i < N/4) sum += arr3[i];
        sum += arr1[i];
    }
    
    global_sum += sum;
}

int main(void) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all test functions */
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
    
    test_mixed_partitions();
    printf("Test 9 (mixed partitions) complete\n");
    
    /* Final checksum */
    int final_sum = 0;
    for (int i = 0; i < 8; i++) {
        final_sum += results[i];
    }
    final_sum += global_sum;
    
    printf("Final checksum: %d\n", final_sum);
    printf("All tests completed.\n");
    
    return 0;
}
