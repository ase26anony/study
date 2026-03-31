/* Test program to cover partition mapping cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 2  /* vectors */
#define N 1024

/* Global arrays to prevent optimization */
int global_3d[G][W][V];
int global_2d[W][V];
int global_1d[N];
volatile int checksum = 0;

/* Function 1: Gang redundant/partitioned */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(global_1d[0:N]) copyout(local_sum) num_gangs(G)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            local_sum += global_1d[i];
        }
    }
    
    checksum += local_sum;
}

/* Function 2: Worker partitioned */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            #pragma acc loop vector
            for (int j = 0; j < V; j++) {
                global_2d[i][j] += i + j;
            }
        }
    }
    
    /* Update checksum */
    int sum = 0;
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            sum += global_2d[i][j];
        }
    }
    checksum += sum;
}

/* Function 3: Vector partitioned */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            global_1d[i] *= 2;
        }
    }
    
    /* Update checksum */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += global_1d[i];
    }
    checksum += sum;
}

/* Function 4: Gang+worker partitioned */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    global_3d[g][w][v] = g * 100 + w * 10 + v;
                }
            }
        }
    }
    
    /* Update checksum */
    int sum = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                sum += global_3d[g][w][v];
            }
        }
    }
    checksum += sum;
}

/* Function 5: Gang+vector partitioned */
void test_gang_vector_partitioned(void) {
    int arr[G][V];
    
    #pragma acc parallel copy(arr[0:G][0:V]) \
                num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang vector
        for (int g = 0; g < G; g++) {
            for (int v = 0; v < V; v++) {
                arr[g][v] = g * 10 + v;
            }
        }
    }
    
    /* Update checksum */
    int sum = 0;
    for (int g = 0; g < G; g++) {
        for (int v = 0; v < V; v++) {
            sum += arr[g][v];
        }
    }
    checksum += sum;
}

/* Function 6: Worker+vector partitioned */
void test_worker_vector_partitioned(void) {
    int arr[W][V];
    
    #pragma acc parallel copy(arr[0:W][0:V]) \
                num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr[w][v] = w * 10 + v;
            }
        }
    }
    
    /* Update checksum */
    int sum = 0;
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            sum += arr[w][v];
        }
    }
    checksum += sum;
}

/* Function 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    global_3d[g][w][v] += 1;
                }
            }
        }
    }
    
    /* Update checksum */
    int sum = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                sum += global_3d[g][w][v];
            }
        }
    }
    checksum += sum;
}

/* Function 8: Mixed partitioning with complex data clauses */
void test_mixed_partitioning(void) {
    int arr1[N], arr2[N], arr3[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
    }
    
    /* Complex data clause with multiple arrays */
    #pragma acc parallel copyin(arr1[0:N], arr2[0:N]) copyout(arr3[0:N]) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 4; j++) {
                arr3[i] += arr1[i] + arr2[i] + j;
            }
        }
    }
    
    /* Update checksum */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr3[i];
    }
    checksum += sum;
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < N; i++) {
        global_1d[i] = i % 100;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            global_2d[i][j] = i * j;
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                global_3d[g][w][v] = 0;
            }
        }
    }
    
    /* Execute all test functions to trigger different partition mappings */
    test_gang_redundant();           /* Should trigger case 0 or 1 */
    test_worker_partitioned();       /* Should trigger case 2 */
    test_vector_partitioned();       /* Should trigger case 4 */
    test_gang_worker_partitioned();  /* Should trigger case 3 */
    test_gang_vector_partitioned();  /* Should trigger case 5 */
    test_worker_vector_partitioned();/* Should trigger case 6 */
    test_fully_partitioned();        /* Should trigger case 7 */
    test_mixed_partitioning();       /* Additional complex case */
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
