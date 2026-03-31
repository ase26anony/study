/* Test program to cover partition mapping strings in GCC's OpenACC neuter/broadcast */
#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 2  /* vectors */
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int output_array[N] = {0};
int arr_3d[G][W][V] = {0};

/* Test 1: Gang redundant (likely case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    int arr[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) arr[i] = i;
    
    #pragma acc parallel copy(arr[0:N]) copy(local_sum) num_gangs(G)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            local_sum += arr[i];
        }
    }
    
    global_sum += local_sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int arr[G][W];
    
    /* Initialize */
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            arr[g][w] = g * W + w;
    
    #pragma acc parallel copy(arr[0:G][0:W]) num_gangs(G) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < W; w++) {
                arr[g][w] *= 2;
            }
        }
    }
    
    /* Store result to prevent optimization */
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            output_array[g * W + w] += arr[g][w];
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[W][V];
    
    /* Initialize */
    for (int w = 0; w < W; w++)
        for (int v = 0; v < V; v++)
            arr[w][v] = w * V + v;
    
    #pragma acc parallel copy(arr[0:W][0:V]) num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector independent
            for (int v = 0; v < V; v++) {
                arr[w][v] += 1;
            }
        }
    }
    
    /* Store result */
    for (int w = 0; w < W; w++)
        for (int v = 0; v < V; v++)
            output_array[w * V + v] += arr[w][v];
}

/* Test 4: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[V];
    
    /* Initialize */
    for (int v = 0; v < V; v++) arr[v] = v;
    
    #pragma acc parallel copy(arr[0:V]) num_gangs(1) num_workers(1) vector_length(V)
    {
        #pragma acc loop vector independent
        for (int v = 0; v < V; v++) {
            arr[v] *= 3;
        }
    }
    
    /* Store result */
    for (int v = 0; v < V; v++)
        output_array[v] += arr[v];
}

/* Test 5: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[G][W];
    
    /* Initialize */
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            arr[g][w] = g * W + w;
    
    #pragma acc parallel copy(arr[0:G][0:W]) num_gangs(G) num_workers(W) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < W; w++) {
                arr[g][w] += g + w;
            }
        }
    }
    
    /* Store result */
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            output_array[g * W + w] += arr[g][w];
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[G][V];
    
    /* Initialize */
    for (int g = 0; g < G; g++)
        for (int v = 0; v < V; v++)
            arr[g][v] = g * V + v;
    
    #pragma acc parallel copy(arr[0:G][0:V]) num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop vector independent
            for (int v = 0; v < V; v++) {
                arr[g][v] -= 1;
            }
        }
    }
    
    /* Store result */
    for (int g = 0; g < G; g++)
        for (int v = 0; v < V; v++)
            output_array[g * V + v] += arr[g][v];
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[W][V];
    
    /* Initialize */
    for (int w = 0; w < W; w++)
        for (int v = 0; v < V; v++)
            arr[w][v] = w * V + v;
    
    #pragma acc parallel copy(arr[0:W][0:V]) num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector independent
            for (int v = 0; v < V; v++) {
                arr[w][v] *= 2;
            }
        }
    }
    
    /* Store result */
    for (int w = 0; w < W; w++)
        for (int v = 0; v < V; v++)
            output_array[w * V + v] += arr[w][v];
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    /* Initialize 3D array */
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            for (int v = 0; v < V; v++)
                arr_3d[g][w][v] = g * W * V + w * V + v;
    
    #pragma acc parallel copy(arr_3d[0:G][0:W][0:V]) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang independent collapse(1)
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent collapse(1)
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < V; v++) {
                    arr_3d[g][w][v] += 5;
                }
            }
        }
    }
    
    /* Store result */
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            for (int v = 0; v < V; v++)
                output_array[(g * W + w) * V + v] += arr_3d[g][w][v];
}

/* Test 9: Combined partitions with collapse */
void test_combined_collapse(void) {
    int arr[G][W][V];
    
    /* Initialize */
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            for (int v = 0; v < V; v++)
                arr[g][w][v] = g * W * V + w * V + v;
    
    #pragma acc parallel copy(arr[0:G][0:W][0:V]) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr[g][w][v] = arr[g][w][v] * 2 + 1;
                }
            }
        }
    }
    
    /* Store result */
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            for (int v = 0; v < V; v++)
                output_array[(g * W + w) * V + v] += arr[g][w][v];
}

int main(void) {
    int checksum = 0;
    
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_vector_partitioned();
    test_gang_worker_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_combined_collapse();
    
    /* Compute checksum of results */
    for (int i = 0; i < N; i++) {
        checksum += output_array[i];
    }
    checksum += global_sum;
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum != 0, computations were performed.\n");
    
    return 0;
}
