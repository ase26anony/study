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
int result_array_1d[N];
int result_array_2d[W][V];
int result_array_3d[G][W][V];

/* Test 1: Likely gang redundant partitioning (case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    int arr[N];
    
    #pragma acc parallel loop gang reduction(+:local_sum) copy(local_sum) copyin(arr[0:N])
    for (int i = 0; i < N; i++) {
        local_sum += arr[i];
    }
    
    global_sum += local_sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int arr[G][W];
    
    #pragma acc parallel loop gang collapse(2) copy(arr[0:G][0:W])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Store results to prevent optimization */
    #pragma acc parallel loop gang collapse(2) copyout(result_array_2d[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            result_array_2d[i][j] = arr[i % G][j % W];
        }
    }
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[W][V];
    
    #pragma acc parallel loop worker collapse(2) copy(arr[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr[i][j] = (i + 1) * (j + 1);
        }
    }
    
    /* Use results */
    #pragma acc parallel loop worker collapse(2) copy(result_array_2d[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            result_array_2d[i][j] += arr[i][j];
        }
    }
}

/* Test 4: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[V];
    
    #pragma acc parallel loop vector copy(arr[0:V])
    for (int i = 0; i < V; i++) {
        arr[i] = i * 10;
    }
    
    /* Store results */
    #pragma acc parallel loop vector copy(result_array_1d[0:V])
    for (int i = 0; i < V; i++) {
        result_array_1d[i] = arr[i];
    }
}

/* Test 5: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[G][W];
    
    #pragma acc parallel loop gang worker collapse(2) copy(arr[0:G][0:W])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            arr[i][j] = i * W + j;
        }
    }
    
    /* Use results in 3D array */
    #pragma acc parallel loop gang worker collapse(3) copy(result_array_3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                result_array_3d[i][j][k] = arr[i][j] + k;
            }
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[G][V];
    
    #pragma acc parallel loop gang vector collapse(2) copy(arr[0:G][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < V; j++) {
            arr[i][j] = i * V + j;
        }
    }
    
    /* Propagate to 3D array */
    #pragma acc parallel loop gang vector collapse(3) copy(result_array_3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                result_array_3d[i][j][k] += arr[i][k];
            }
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[W][V];
    
    #pragma acc parallel loop worker vector collapse(2) copy(arr[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr[i][j] = i * V + j + 100;
        }
    }
    
    /* Update 2D results */
    #pragma acc parallel loop worker vector collapse(2) copy(result_array_2d[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            result_array_2d[i][j] += arr[i][j];
        }
    }
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int arr[G][W][V];
    
    #pragma acc parallel loop gang worker vector collapse(3) copy(arr[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr[i][j][k] = i * W * V + j * V + k;
            }
        }
    }
    
    /* Final update to global 3D array */
    #pragma acc parallel loop gang worker vector collapse(3) copy(result_array_3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                result_array_3d[i][j][k] += arr[i][j][k];
            }
        }
    }
}

int main(void) {
    printf("Starting OpenACC partition mapping tests...\n");
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) result_array_1d[i] = 0;
    for (int i = 0; i < W; i++) 
        for (int j = 0; j < V; j++) 
            result_array_2d[i][j] = 0;
    for (int i = 0; i < G; i++)
        for (int j = 0; j < W; j++)
            for (int k = 0; k < V; k++)
                result_array_3d[i][j][k] = 0;
    
    /* Execute tests targeting different partition mappings */
    test_gang_redundant();          /* Case 0 */
    test_gang_partitioned();        /* Case 1 */
    test_worker_partitioned();      /* Case 2 */
    test_vector_partitioned();      /* Case 4 */
    test_gang_worker_partitioned(); /* Case 3 */
    test_gang_vector_partitioned(); /* Case 5 */
    test_worker_vector_partitioned(); /* Case 6 */
    test_fully_partitioned();       /* Case 7 */
    
    /* Compute checksum to ensure all computations executed */
    int checksum = global_sum;
    for (int i = 0; i < V; i++) checksum += result_array_1d[i];
    for (int i = 0; i < W; i++)
        for (int j = 0; j < V; j++)
            checksum += result_array_2d[i][j];
    for (int i = 0; i < G; i++)
        for (int j = 0; j < W; j++)
            for (int k = 0; k < V; k++)
                checksum += result_array_3d[i][j][k];
    
    printf("Final checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    return 0;
}
