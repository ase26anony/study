/* Test program to cover partition mapping in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define G 8   /* gangs */
#define W 4   /* workers */
#define V 32  /* vector length */
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int output_array[N];
int arr_3d[G][W][V];
int arr_2d[W][V];
int arr_1d[N];

/* Test 1: Gang redundant partition (likely case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(arr_1d[0:N]) copyout(output_array[0:N]) reduction(+:local_sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            output_array[i] = arr_1d[i] + 1;
            local_sum += output_array[i];
        }
    }
    
    global_sum += local_sum;
}

/* Test 2: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int partial_sums[W] = {0};
    
    #pragma acc parallel copyin(arr_2d[0:W][0:V]) copyout(partial_sums[0:W])
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            int worker_sum = 0;
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                worker_sum += arr_2d[w][v];
            }
            partial_sums[w] = worker_sum;
        }
    }
    
    for (int w = 0; w < W; w++) {
        global_sum += partial_sums[w];
    }
}

/* Test 3: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(arr_1d[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr_1d[i] *= 2;  /* Vector operation */
        }
    }
    
    /* Verify some results */
    int check = 0;
    for (int i = 0; i < 10; i++) {
        check += arr_1d[i];
    }
    global_sum += check;
}

/* Test 4: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int gang_results[G] = {0};
    
    #pragma acc parallel copyin(arr_3d[0:G][0:W][0:V]) copyout(gang_results[0:G]) num_gangs(G)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            int gang_sum = 0;
            #pragma acc loop worker vector collapse(2)
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    gang_sum += arr_3d[g][w][v];
                }
            }
            gang_results[g] = gang_sum;
        }
    }
    
    for (int g = 0; g < G; g++) {
        global_sum += gang_results[g];
    }
}

/* Test 5: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int gw_results[G][W];
    
    #pragma acc parallel copyin(arr_3d[0:G][0:W][0:V]) copyout(gw_results[0:G][0:W]) \
                num_gangs(G) num_workers(W)
    {
        #pragma acc loop gang worker independent collapse(2)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                int sum = 0;
                #pragma acc loop vector
                for (int v = 0; v < V; v++) {
                    sum += arr_3d[g][w][v];
                }
                gw_results[g][w] = sum;
            }
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            global_sum += gw_results[g][w];
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(arr_2d[0:W][0:V]) num_gangs(2) vector_length(V)
    {
        #pragma acc loop gang vector independent collapse(2)
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr_2d[w][v] += w + v;
            }
        }
    }
    
    /* Quick checksum */
    int check = 0;
    #pragma acc parallel copyin(arr_2d[0:W][0:V]) reduction(+:check)
    {
        #pragma acc loop gang worker vector collapse(2)
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                check += arr_2d[w][v];
            }
        }
    }
    global_sum += check;
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int wv_results[W][V];
    
    #pragma acc parallel copyin(arr_2d[0:W][0:V]) copyout(wv_results[0:W][0:V]) \
                num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector independent collapse(2)
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                wv_results[w][v] = arr_2d[w][v] * 3;
            }
        }
    }
    
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            global_sum += wv_results[w][v];
        }
    }
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int result = 0;
    
    #pragma acc parallel copyin(arr_3d[0:G][0:W][0:V]) reduction(+:result) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector independent collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    result += arr_3d[g][w][v];
                }
            }
        }
    }
    
    global_sum += result;
}

/* Initialize test data */
void init_data(void) {
    /* Initialize 1D array */
    for (int i = 0; i < N; i++) {
        arr_1d[i] = i % 100;
    }
    
    /* Initialize 2D array */
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr_2d[w][v] = (w * 10 + v) % 100;
        }
    }
    
    /* Initialize 3D array */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr_3d[g][w][v] = (g * 100 + w * 10 + v) % 100;
            }
        }
    }
}

int main(void) {
    /* Initialize test data */
    init_data();
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Execute all test patterns to trigger different partition mappings */
    test_gang_redundant();
    printf("  Test 1 (gang redundant) completed\n");
    
    test_worker_partitioned();
    printf("  Test 2 (worker partitioned) completed\n");
    
    test_vector_partitioned();
    printf("  Test 3 (vector partitioned) completed\n");
    
    test_gang_partitioned();
    printf("  Test 4 (gang partitioned) completed\n");
    
    test_gang_worker_partitioned();
    printf("  Test 5 (gang+worker partitioned) completed\n");
    
    test_gang_vector_partitioned();
    printf("  Test 6 (gang+vector partitioned) completed\n");
    
    test_worker_vector_partitioned();
    printf("  Test 7 (worker+vector partitioned) completed\n");
    
    test_fully_partitioned();
    printf("  Test 8 (fully partitioned) completed\n");
    
    /* Final checksum to ensure all computations happened */
    printf("Final checksum: %d\n", global_sum);
    
    return 0;
}
