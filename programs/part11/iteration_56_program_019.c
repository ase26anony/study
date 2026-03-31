/* Test program to cover partition mapping strings in GCC's OpenACC neuter/broadcast pass.
   Designed to trigger all cases (0-7) of the partition code to string mapping function. */

#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to store results and prevent optimization */
volatile int global_sum = 0;
int result_1d[N];
int result_2d[W][V];
int result_3d[G][W][V];

/* Test 1: Likely gang redundant (case 0) - scalar reduction across gangs */
void test_gang_redundant(void) {
    int sum = 0;
    int arr[N];
    
    for (int i = 0; i < N; i++) arr[i] = i % 100;
    
    #pragma acc parallel copyin(arr[0:N]) copy(sum) num_gangs(4)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
    }
    
    global_sum += sum;
}

/* Test 2: Worker partitioned (case 2) - each worker processes a row */
void test_worker_partitioned(void) {
    int arr[W][V];
    
    for (int i = 0; i < W; i++)
        for (int j = 0; j < V; j++)
            arr[i][j] = i * 10 + j;
    
    #pragma acc parallel copy(arr[0:W][0:V]) copyout(result_2d[0:W][0:V]) \
                num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            #pragma acc loop vector
            for (int j = 0; j < V; j++) {
                result_2d[i][j] = arr[i][j] + 1;
            }
        }
    }
}

/* Test 3: Vector partitioned (case 4) - vector operations */
void test_vector_partitioned(void) {
    int a[N], b[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
    }
    
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(result_1d[0:N]) \
                vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result_1d[i] = a[i] + b[i];
        }
    }
}

/* Test 4: Combined partitions - targets cases 1,3,5,6,7 */
void test_combined_partitions(void) {
    int arr[G][W][V];
    
    /* Initialize 3D array */
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            for (int v = 0; v < V; v++)
                arr[g][w][v] = g * 100 + w * 10 + v;
    
    /* Case 1/3/5/6/7: Various partition combinations through nested loops */
    #pragma acc parallel copy(arr[0:G][0:W][0:V]) copyout(result_3d[0:G][0:W][0:V]) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        /* Triple nested loop with collapse - may generate combined partitions */
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    result_3d[g][w][v] = arr[g][w][v] * 2;
                }
            }
        }
    }
    
    /* Additional explicit partition tests */
    int temp[G][W];
    
    #pragma acc parallel copy(temp[0:G][0:W]) num_gangs(G) num_workers(W)
    {
        /* Gang+worker partitioned (case 3) */
        #pragma acc loop gang worker
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                temp[g][w] = g + w;
            }
        }
    }
    
    int temp2[G][V];
    
    #pragma acc parallel copy(temp2[0:G][0:V]) num_gangs(G) vector_length(V)
    {
        /* Gang+vector partitioned (case 5) */
        #pragma acc loop gang vector
        for (int g = 0; g < G; g++) {
            for (int v = 0; v < V; v++) {
                temp2[g][v] = g * v;
            }
        }
    }
}

/* Test 5: Explicit gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int arr[G][N/G];
    
    #pragma acc parallel copy(arr[0:G][0:N/G]) num_gangs(G)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            for (int i = 0; i < N/G; i++) {
                arr[g][i] = g * 1000 + i;
            }
        }
    }
}

/* Test 6: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[W][V][4];
    
    #pragma acc parallel copy(arr[0:W][0:V][0:4]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                for (int k = 0; k < 4; k++) {
                    arr[w][v][k] = w * v * k;
                }
            }
        }
    }
}

int main(void) {
    printf("Starting OpenACC partition mapping tests...\n");
    
    /* Execute all test functions to trigger various partition mappings */
    test_gang_redundant();
    printf("Test 1 (gang redundant) completed\n");
    
    test_gang_partitioned();
    printf("Test 2 (gang partitioned) completed\n");
    
    test_worker_partitioned();
    printf("Test 3 (worker partitioned) completed\n");
    
    test_vector_partitioned();
    printf("Test 4 (vector partitioned) completed\n");
    
    test_worker_vector_partitioned();
    printf("Test 5 (worker+vector partitioned) completed\n");
    
    test_combined_partitions();
    printf("Test 6 (combined partitions) completed\n");
    
    /* Compute checksum to ensure all computations were performed */
    int checksum = global_sum;
    for (int i = 0; i < N; i++) checksum += result_1d[i];
    for (int i = 0; i < W; i++)
        for (int j = 0; j < V; j++)
            checksum += result_2d[i][j];
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            for (int v = 0; v < V; v++)
                checksum += result_3d[g][w][v];
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed. If compiled with -fopenacc and offload enabled,\n");
    printf("this should trigger all partition mapping cases (0-7).\n");
    
    return 0;
}
