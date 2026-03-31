/* test_partition_coverage.c - Cover all partition mapping cases in GCC's OpenACC */
#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 32 /* vector length */

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int results[8] = {0};

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant(void) {
    int arr[100];
    int sum = 0;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(arr[0:100])
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
        sum += arr[i];
    }
    
    results[0] = sum;
    global_sum += sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int arr[G][100];
    
    #pragma acc parallel loop gang copy(arr[0:G][0:100])
    for (int g = 0; g < G; g++) {
        for (int i = 0; i < 100; i++) {
            arr[g][i] = g * 100 + i;
        }
    }
    
    int sum = 0;
    for (int g = 0; g < G; g++) {
        for (int i = 0; i < 100; i++) {
            sum += arr[g][i];
        }
    }
    results[1] = sum;
    global_sum += sum;
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[W][50];
    
    #pragma acc parallel loop worker copy(arr[0:W][0:50])
    for (int w = 0; w < W; w++) {
        for (int i = 0; i < 50; i++) {
            arr[w][i] = w * 50 + i;
        }
    }
    
    int sum = 0;
    for (int w = 0; w < W; w++) {
        for (int i = 0; i < 50; i++) {
            sum += arr[w][i];
        }
    }
    results[2] = sum;
    global_sum += sum;
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[G][W][20];
    
    #pragma acc parallel loop gang worker collapse(2) copy(arr[0:G][0:W][0:20])
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int i = 0; i < 20; i++) {
                arr[g][w][i] = (g * W + w) * 20 + i;
            }
        }
    }
    
    int sum = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int i = 0; i < 20; i++) {
                sum += arr[g][w][i];
            }
        }
    }
    results[3] = sum;
    global_sum += sum;
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[V];
    
    #pragma acc parallel loop vector copy(arr[0:V])
    for (int v = 0; v < V; v++) {
        arr[v] = v * 2;
    }
    
    int sum = 0;
    for (int v = 0; v < V; v++) {
        sum += arr[v];
    }
    results[4] = sum;
    global_sum += sum;
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[G][V];
    
    #pragma acc parallel loop gang vector collapse(2) copy(arr[0:G][0:V])
    for (int g = 0; g < G; g++) {
        for (int v = 0; v < V; v++) {
            arr[g][v] = g * V + v;
        }
    }
    
    int sum = 0;
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
    
    #pragma acc parallel loop worker vector collapse(2) copy(arr[0:W][0:V])
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr[w][v] = w * V + v;
        }
    }
    
    int sum = 0;
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
                arr[g][w][v] = ((g * W + w) * V + v) % 100;
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

/* Test 9: Mixed clauses to trigger internal mapping */
void test_mixed_clauses(void) {
    int arr1[100], arr2[100], arr3[100];
    
    /* Multiple data clauses with different shapes */
    #pragma acc parallel copy(arr1[0:100]) copyin(arr2[0:100]) copyout(arr3[0:100]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang
        for (int i = 0; i < 100; i++) {
            arr3[i] = arr1[i] + arr2[i];
        }
    }
    
    /* Another region with explicit partition clauses */
    int arr4[G][W][V];
    #pragma acc parallel copy(arr4[0:G][0:W][0:V])
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector
                for (int v = 0; v < V; v++) {
                    arr4[g][w][v] = g + w + v;
                }
            }
        }
    }
}

int main(void) {
    printf("Testing OpenACC partition mapping coverage...\n");
    
    /* Initialize some data */
    int init_arr[G][W][V];
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                init_arr[g][w][v] = 1;
            }
        }
    }
    
    /* Execute all test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_mixed_clauses();
    
    /* Compute final checksum */
    int final_sum = global_sum;
    for (int i = 0; i < 8; i++) {
        final_sum += results[i];
    }
    
    printf("Final checksum: %d\n", final_sum);
    printf("Test completed.\n");
    
    return 0;
}
