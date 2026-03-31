/* Test program to cover partition mapping in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int results[8] = {0};

/* Test 1: Gang redundant partitioning (case 0) */
void test_gang_redundant(void) {
    int arr[N];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i % 100;
    }
    
    #pragma acc parallel copy(arr[0:N]) copy(sum) num_gangs(4)
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
    int partial_sum = 0;
    
    /* Initialize 2D array */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            arr[g][w] = g * W + w;
        }
    }
    
    #pragma acc parallel copy(arr[0:G][0:W]) copy(partial_sum) \
                num_gangs(G) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            int gang_sum = 0;
            #pragma acc loop worker reduction(+:gang_sum)
            for (int w = 0; w < W; w++) {
                gang_sum += arr[g][w];
            }
            #pragma acc atomic
            partial_sum += gang_sum;
        }
    }
    
    results[1] = partial_sum;
    global_sum += partial_sum;
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[W][V];
    int sum = 0;
    
    /* Initialize array */
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr[w][v] = w * V + v;
        }
    }
    
    #pragma acc parallel copy(arr[0:W][0:V]) copy(sum) \
                num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < W; w++) {
            int worker_sum = 0;
            #pragma acc loop vector reduction(+:worker_sum)
            for (int v = 0; v < V; v++) {
                worker_sum += arr[w][v];
            }
            #pragma acc atomic
            sum += worker_sum;
        }
    }
    
    results[2] = sum;
    global_sum += sum;
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[G][W];
    int sum = 0;
    
    /* Initialize 2D array */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            arr[g][w] = g * W + w;
        }
    }
    
    #pragma acc parallel copy(arr[0:G][0:W]) copy(sum) \
                num_gangs(G) num_workers(W) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent reduction(+:sum)
            for (int w = 0; w < W; w++) {
                sum += arr[g][w];
            }
        }
    }
    
    results[3] = sum;
    global_sum += sum;
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[V];
    int sum = 0;
    
    /* Initialize array */
    for (int v = 0; v < V; v++) {
        arr[v] = v + 1;
    }
    
    #pragma acc parallel copy(arr[0:V]) copy(sum) \
                num_gangs(1) num_workers(1) vector_length(V)
    {
        #pragma acc loop vector independent reduction(+:sum)
        for (int v = 0; v < V; v++) {
            sum += arr[v];
        }
    }
    
    results[4] = sum;
    global_sum += sum;
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[G][V];
    int sum = 0;
    
    /* Initialize 2D array */
    for (int g = 0; g < G; g++) {
        for (int v = 0; v < V; v++) {
            arr[g][v] = g * V + v;
        }
    }
    
    #pragma acc parallel copy(arr[0:G][0:V]) copy(sum) \
                num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop vector independent reduction(+:sum)
            for (int v = 0; v < V; v++) {
                sum += arr[g][v];
            }
        }
    }
    
    results[5] = sum;
    global_sum += sum;
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[W][V];
    int sum = 0;
    
    /* Initialize 2D array */
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr[w][v] = w * V + v;
        }
    }
    
    #pragma acc parallel copy(arr[0:W][0:V]) copy(sum) \
                num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector independent reduction(+:sum)
            for (int v = 0; v < V; v++) {
                sum += arr[w][v];
            }
        }
    }
    
    results[6] = sum;
    global_sum += sum;
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int arr[G][W][V];
    int sum = 0;
    
    /* Initialize 3D array */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr[g][w][v] = g * W * V + w * V + v;
            }
        }
    }
    
    #pragma acc parallel copy(arr[0:G][0:W][0:V]) copy(sum) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector independent reduction(+:sum)
                for (int v = 0; v < V; v++) {
                    sum += arr[g][w][v];
                }
            }
        }
    }
    
    results[7] = sum;
    global_sum += sum;
}

/* Test 9: Combined partitions using collapse */
void test_combined_collapse(void) {
    int arr[G][W][V];
    int sum = 0;
    
    /* Initialize 3D array */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr[g][w][v] = 1;
            }
        }
    }
    
    /* This should trigger various partition mappings internally */
    #pragma acc parallel copy(arr[0:G][0:W][0:V]) copy(sum) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3) reduction(+:sum)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    sum += arr[g][w][v];
                }
            }
        }
    }
    
    global_sum += sum;
}

/* Test 10: Data regions with explicit mapping */
void test_data_regions(void) {
    int arr1[N], arr2[N], arr3[N];
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i % 10;
        arr2[i] = (i + 1) % 10;
        arr3[i] = 0;
    }
    
    /* Data region with multiple arrays */
    #pragma acc data copyin(arr1[0:N], arr2[0:N]) copyout(arr3[0:N])
    {
        #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                arr3[i] = arr1[i] + arr2[i];
            }
        }
        
        #pragma acc parallel loop reduction(+:sum) \
                    num_gangs(4) num_workers(4) vector_length(16)
        for (int i = 0; i < N; i++) {
            sum += arr3[i];
        }
    }
    
    global_sum += sum;
}

int main(void) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_combined_collapse();
    test_data_regions();
    
    /* Print results to prevent dead code elimination */
    printf("Results: ");
    for (int i = 0; i < 8; i++) {
        printf("%d ", results[i]);
    }
    printf("\nGlobal sum: %d\n", (int)global_sum);
    
    return 0;
}
