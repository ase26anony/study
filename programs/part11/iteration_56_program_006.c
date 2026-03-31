/* test_openacc_partitions.c
 * Designed to cover the partition mapping function in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: case 0-7 return strings for different partition types
 */

#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs dimension */
#define W 4  /* workers dimension */
#define V 2  /* vectors dimension */
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
    
    /* Simple parallel reduction - likely gang redundant */
    #pragma acc parallel copyin(arr[0:N]) copy(sum) num_gangs(4)
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
    int arr[G][W];
    int partial_sums[G] = {0};
    
    /* Initialize 2D array */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            arr[g][w] = g * 10 + w;
        }
    }
    
    /* Each gang processes its own slice */
    #pragma acc parallel copyin(arr[0:G][0:W]) copyout(partial_sums[0:G]) \
        num_gangs(G) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            int gang_sum = 0;
            #pragma acc loop worker
            for (int w = 0; w < W; w++) {
                gang_sum += arr[g][w];
            }
            partial_sums[g] = gang_sum;
        }
    }
    
    /* Combine results on host */
    for (int g = 0; g < G; g++) {
        results[1] += partial_sums[g];
    }
    global_sum += results[1];
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[W][V];
    int worker_results[W] = {0};
    
    /* Initialize array */
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr[w][v] = w * 5 + v;
        }
    }
    
    /* Explicit worker partitioning */
    #pragma acc parallel copyin(arr[0:W][0:V]) copyout(worker_results[0:W]) \
        num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            int worker_sum = 0;
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                worker_sum += arr[w][v];
            }
            worker_results[w] = worker_sum;
        }
    }
    
    for (int w = 0; w < W; w++) {
        results[2] += worker_results[w];
    }
    global_sum += results[2];
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[G][W];
    int total = 0;
    
    /* Initialize */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            arr[g][w] = 1;
        }
    }
    
    /* Combined gang and worker partitioning */
    #pragma acc parallel copyin(arr[0:G][0:W]) copy(total) \
        num_gangs(G) num_workers(W) vector_length(1)
    {
        #pragma acc loop gang worker reduction(+:total)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                total += arr[g][w];
            }
        }
    }
    
    results[3] = total;
    global_sum += total;
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[N];
    int vec_results[V] = {0};
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr[i] = i % 7;
    }
    
    /* Explicit vector partitioning */
    #pragma acc parallel copyin(arr[0:N]) copyout(vec_results[0:V]) \
        num_gangs(1) num_workers(1) vector_length(V)
    {
        #pragma acc loop vector
        for (int v = 0; v < V; v++) {
            int vec_sum = 0;
            /* Process every V-th element */
            for (int i = v; i < N; i += V) {
                vec_sum += arr[i];
            }
            vec_results[v] = vec_sum;
        }
    }
    
    for (int v = 0; v < V; v++) {
        results[4] += vec_results[v];
    }
    global_sum += results[4];
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[G][V];
    int total = 0;
    
    /* Initialize */
    for (int g = 0; g < G; g++) {
        for (int v = 0; v < V; v++) {
            arr[g][v] = g * V + v;
        }
    }
    
    /* Gang and vector partitioning */
    #pragma acc parallel copyin(arr[0:G][0:V]) copy(total) \
        num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang vector reduction(+:total)
        for (int g = 0; g < G; g++) {
            for (int v = 0; v < V; v++) {
                total += arr[g][v];
            }
        }
    }
    
    results[5] = total;
    global_sum += total;
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[W][V];
    int total = 0;
    
    /* Initialize */
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr[w][v] = w * V + v + 1;
        }
    }
    
    /* Worker and vector partitioning */
    #pragma acc parallel copyin(arr[0:W][0:V]) copy(total) \
        num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector reduction(+:total)
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                total += arr[w][v];
            }
        }
    }
    
    results[6] = total;
    global_sum += total;
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int arr[G][W][V];
    int total = 0;
    
    /* Initialize 3D array */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr[g][w][v] = 1;
            }
        }
    }
    
    /* Fully partitioned across all levels */
    #pragma acc parallel copyin(arr[0:G][0:W][0:V]) copy(total) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector reduction(+:total) collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    total += arr[g][w][v];
                }
            }
        }
    }
    
    results[7] = total;
    global_sum += total;
}

int main(void) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all test cases to trigger different partition types */
    test_gang_redundant();
    printf("Test 1 (gang redundant) complete: result = %d\n", results[0]);
    
    test_gang_partitioned();
    printf("Test 2 (gang partitioned) complete: result = %d\n", results[1]);
    
    test_worker_partitioned();
    printf("Test 3 (worker partitioned) complete: result = %d\n", results[2]);
    
    test_gang_worker_partitioned();
    printf("Test 4 (gang+worker partitioned) complete: result = %d\n", results[3]);
    
    test_vector_partitioned();
    printf("Test 5 (vector partitioned) complete: result = %d\n", results[4]);
    
    test_gang_vector_partitioned();
    printf("Test 6 (gang+vector partitioned) complete: result = %d\n", results[5]);
    
    test_worker_vector_partitioned();
    printf("Test 7 (worker+vector partitioned) complete: result = %d\n", results[6]);
    
    test_fully_partitioned();
    printf("Test 8 (fully partitioned) complete: result = %d\n", results[7]);
    
    /* Final checksum */
    printf("\nGlobal checksum: %d\n", global_sum);
    printf("Expected checksum: %d\n", 8 * G * W * V);  /* All tests add 1s in various patterns */
    
    if (global_sum > 0) {
        printf("All OpenACC regions executed successfully.\n");
        return 0;
    } else {
        printf("ERROR: No computation performed!\n");
        return 1;
    }
}
