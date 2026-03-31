/* Test program to cover partition mapping cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 2  /* vectors */
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int results[8] = {0};  /* Store results from different test cases */

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant(void) {
    int arr[N];
    int sum = 0;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(arr[0:N])
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
    
    #pragma acc parallel copy(arr[0:G][0:W]) num_gangs(G) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker
            for (int w = 0; w < W; w++) {
                arr[g][w] = g * 100 + w;
            }
        }
    }
    
    int sum = 0;
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
    
    #pragma acc parallel copy(arr[0:W][0:V]) num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                arr[w][v] = w * 10 + v;
            }
        }
    }
    
    int sum = 0;
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
    
    #pragma acc parallel copy(arr[0:G][0:W][0:V]) num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector
                for (int v = 0; v < V; v++) {
                    arr[g][w][v] = g * 1000 + w * 100 + v;
                }
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
    results[3] = sum;
    global_sum += sum;
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[N];
    
    #pragma acc parallel loop vector copy(arr[0:N]) num_gangs(1) num_workers(1) vector_length(V)
    for (int i = 0; i < N; i++) {
        arr[i] = i * 2;
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    results[4] = sum;
    global_sum += sum;
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[G][V];
    
    #pragma acc parallel copy(arr[0:G][0:V]) num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                arr[g][v] = g * 100 + v;
            }
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
    
    #pragma acc parallel copy(arr[0:W][0:V]) num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                arr[w][v] = w * 50 + v * 2;
            }
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
    
    #pragma acc parallel copy(arr[0:G][0:W][0:V]) num_gangs(G) num_workers(W) vector_length(V)
    #pragma acc loop gang worker vector collapse(3)
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr[g][w][v] = g * W * V + w * V + v;
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

/* Test 9: Mixed partition types with data clauses */
void test_mixed_partitions(void) {
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
    }
    
    /* Different data clauses to trigger various partition decisions */
    #pragma acc parallel loop gang copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    #pragma acc parallel loop worker copy(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] *= 2;
    }
    
    #pragma acc parallel loop vector copy(c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] += 1;
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    global_sum += sum;
}

int main(void) {
    printf("Testing OpenACC partition mapping cases...\n");
    
    /* Execute all test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_mixed_partitions();
    
    /* Compute final checksum */
    int final_sum = global_sum;
    for (int i = 0; i < 8; i++) {
        final_sum += results[i];
    }
    
    printf("Final checksum: %d\n", final_sum);
    printf("All tests completed.\n");
    
    return 0;
}
