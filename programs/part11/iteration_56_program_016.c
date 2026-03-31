/* Test program to cover partition mapping strings in GCC's OpenACC neuter/broadcast pass.
   Designed to trigger all partition codes (0-7) through various OpenACC constructs. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define G 8   /* gangs */
#define W 4   /* workers */
#define V 32  /* vector length */

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int arr3d[G][W][V];
int arr2d[W][V];
int arr1d[V];

/* Function 1: Likely triggers gang redundant (case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(arr1d[0:V]) copy(local_sum) num_gangs(G)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < V; i++) {
            local_sum += arr1d[i];
        }
    }
    
    global_sum += local_sum;
}

/* Function 2: Likely triggers gang partitioned (case 1) */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector
                for (int v = 0; v < V; v++) {
                    /* Each gang processes different workers */
                    if (g < W) arr2d[w][v] += g;
                }
            }
        }
    }
}

/* Function 3: Likely triggers worker partitioned (case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                arr2d[w][v] += w * v;
            }
        }
    }
}

/* Function 4: Likely triggers gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = g * 100 + w * 10 + v;
                }
            }
        }
    }
}

/* Function 5: Likely triggers vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:V]) num_gangs(1) num_workers(1) vector_length(V)
    {
        #pragma acc loop vector
        for (int v = 0; v < V; v++) {
            arr1d[v] = v * 2;
        }
    }
}

/* Function 6: Likely triggers gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:G][0:V]) num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                arr2d[g][v] = g * V + v;
            }
        }
    }
}

/* Function 7: Likely triggers worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                arr2d[w][v] += w + v;
            }
        }
    }
}

/* Function 8: Likely triggers fully partitioned (case 7) */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] += 1;
                }
            }
        }
    }
}

/* Function 9: Mixed directives to cover edge cases */
void test_mixed_partitions(void) {
    /* Combined parallel and kernels regions */
    #pragma acc data copy(arr3d[0:G][0:W][0:V])
    {
        #pragma acc kernels
        {
            #pragma acc loop gang
            for (int g = 0; g < G; g++) {
                #pragma acc loop worker
                for (int w = 0; w < W; w++) {
                    #pragma acc loop vector
                    for (int v = 0; v < V; v++) {
                        arr3d[g][w][v] *= 2;
                    }
                }
            }
        }
        
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector collapse(2)
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr2d[w][v] = w * v;
                }
            }
        }
    }
}

int main(void) {
    /* Initialize arrays */
    for (int v = 0; v < V; v++) {
        arr1d[v] = v;
        for (int w = 0; w < W; w++) {
            arr2d[w][v] = w * 10 + v;
            for (int g = 0; g < G; g++) {
                arr3d[g][w][v] = g * 100 + w * 10 + v;
            }
        }
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* Execute all test functions to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_mixed_partitions();
    
    /* Compute checksum to ensure all computations executed */
    int checksum = global_sum;
    for (int v = 0; v < V; v++) {
        checksum += arr1d[v];
        for (int w = 0; w < W; w++) {
            checksum += arr2d[w][v];
            for (int g = 0; g < G; g++) {
                checksum += arr3d[g][w][v];
            }
        }
    }
    
    printf("Test completed. Checksum: %d\n", checksum);
    printf("If compiled with -fopenacc and offload enabled, this should\n");
    printf("exercise the partition mapping strings in omp-oacc-neuter-broadcast.cc\n");
    
    return 0;
}
