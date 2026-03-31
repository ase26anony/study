/* Test program to cover partition mapping in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 2  /* vectors */
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int arr3d[G][W][V];
int arr2d[W][V];
int arr1d[N];

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant() {
    int local_sum = 0;
    
    #pragma acc parallel copyin(arr1d[0:N]) copyout(arr1d[0:N]) reduction(+:local_sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] = i * 2;
            local_sum += arr1d[i];
        }
    }
    
    global_sum += local_sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned() {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(W) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int i = 0; i < W; i++) {
            #pragma acc loop worker independent
            for (int j = 0; j < V; j++) {
                arr2d[i][j] = i * 100 + j;
            }
        }
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            sum += arr2d[i][j];
        }
    }
    global_sum += sum;
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned() {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker independent
        for (int i = 0; i < W; i++) {
            #pragma acc loop vector independent
            for (int j = 0; j < V; j++) {
                arr2d[i][j] += 1;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            sum += arr2d[i][j];
        }
    }
    global_sum += sum;
}

/* Test 4: Vector partitioned (case 4) */
void test_vector_partitioned() {
    #pragma acc parallel copy(arr1d[0:N]) num_gangs(1) num_workers(1) vector_length(256)
    {
        #pragma acc loop vector independent
        for (int i = 0; i < N; i++) {
            arr1d[i] = arr1d[i] * 3 + 1;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1d[i];
    }
    global_sum += sum;
}

/* Test 5: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned() {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G) num_workers(W) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = g * 1000 + w * 100 + v;
                }
            }
        }
    }
    
    int sum = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                sum += arr3d[g][w][v];
            }
        }
    }
    global_sum += sum;
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned() {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] += v * 10;
                }
            }
        }
    }
    
    int sum = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                sum += arr3d[g][w][v];
            }
        }
    }
    global_sum += sum;
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned() {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector independent
            for (int v = 0; v < V; v++) {
                arr2d[w][v] = w * v * 7;
            }
        }
    }
    
    int sum = 0;
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            sum += arr2d[w][v];
        }
    }
    global_sum += sum;
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned() {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = (g + w + v) * 2;
                }
            }
        }
    }
    
    int sum = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                sum += arr3d[g][w][v];
            }
        }
    }
    global_sum += sum;
}

int main() {
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = 0;
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr3d[g][w][v] = 0;
            }
        }
    }
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_vector_partitioned();
    test_gang_worker_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    printf("Final checksum: %d\n", global_sum);
    printf("All OpenACC partition tests completed.\n");
    
    return 0;
}
