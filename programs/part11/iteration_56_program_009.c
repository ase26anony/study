/* Test program to cover partition mapping cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 2  /* vectors */
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int results[8] = {0};  /* Store results from different tests */

/* Test 0: Gang redundant - scalar reduction */
void test_gang_redundant(void) {
    int arr[N];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) arr[i] = i % 100;
    
    #pragma acc parallel copyin(arr[0:N]) copy(sum) num_gangs(G)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
    }
    
    results[0] = sum;
    global_sum += sum;
}

/* Test 1: Gang partitioned - 1D array partitioned across gangs */
void test_gang_partitioned(void) {
    int arr[N];
    int partial_sums[G] = {0};
    
    for (int i = 0; i < N; i++) arr[i] = i % 100;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(partial_sums[0:G]) \
        num_gangs(G) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            int gid = __pgi_gangidx();
            if (gid < G) partial_sums[gid] += arr[i];
        }
    }
    
    int total = 0;
    for (int i = 0; i < G; i++) total += partial_sums[i];
    results[1] = total;
    global_sum += total;
}

/* Test 2: Worker partitioned - 2D array with worker-level parallelism */
void test_worker_partitioned(void) {
    int arr[W][N/W];
    int worker_results[W] = {0};
    
    for (int w = 0; w < W; w++) {
        for (int i = 0; i < N/W; i++) {
            arr[w][i] = (w * 100 + i) % 100;
        }
    }
    
    #pragma acc parallel copyin(arr[0:W][0:N/W]) copyout(worker_results[0:W]) \
        num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            int wid = __pgi_workeridx();
            if (wid < W) {
                for (int i = 0; i < N/W; i++) {
                    worker_results[wid] += arr[wid][i];
                }
            }
        }
    }
    
    int total = 0;
    for (int w = 0; w < W; w++) total += worker_results[w];
    results[2] = total;
    global_sum += total;
}

/* Test 3: Gang+worker partitioned - 3D array with gang and worker dims */
void test_gang_worker_partitioned(void) {
    int arr[G][W][N/(G*W)];
    int gw_results[G][W];
    
    /* Initialize 3D array */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int i = 0; i < N/(G*W); i++) {
                arr[g][w][i] = (g * 1000 + w * 100 + i) % 100;
            }
            gw_results[g][w] = 0;
        }
    }
    
    #pragma acc parallel copyin(arr[0:G][0:W][0:N/(G*W)]) \
        copyout(gw_results[0:G][0:W]) \
        num_gangs(G) num_workers(W) vector_length(1)
    {
        #pragma acc loop gang worker collapse(2)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                int gid = __pgi_gangidx();
                int wid = __pgi_workeridx();
                if (gid < G && wid < W) {
                    for (int i = 0; i < N/(G*W); i++) {
                        gw_results[gid][wid] += arr[gid][wid][i];
                    }
                }
            }
        }
    }
    
    int total = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            total += gw_results[g][w];
        }
    }
    results[3] = total;
    global_sum += total;
}

/* Test 4: Vector partitioned - vector operations */
void test_vector_partitioned(void) {
    int a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = i % 100;
        b[i] = (i * 2) % 100;
    }
    
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(1) num_workers(1) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) sum += c[i];
    results[4] = sum;
    global_sum += sum;
}

/* Test 5: Gang+vector partitioned - 2D with gang and vector */
void test_gang_vector_partitioned(void) {
    int arr[G][N/G];
    int gv_results[G] = {0};
    
    for (int g = 0; g < G; g++) {
        for (int i = 0; i < N/G; i++) {
            arr[g][i] = (g * 100 + i) % 100;
        }
    }
    
    #pragma acc parallel copyin(arr[0:G][0:N/G]) copyout(gv_results[0:G]) \
        num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang vector collapse(2)
        for (int g = 0; g < G; g++) {
            for (int i = 0; i < N/G; i++) {
                int gid = __pgi_gangidx();
                if (gid < G) {
                    gv_results[gid] += arr[gid][i];
                }
            }
        }
    }
    
    int total = 0;
    for (int g = 0; g < G; g++) total += gv_results[g];
    results[5] = total;
    global_sum += total;
}

/* Test 6: Worker+vector partitioned - 2D with worker and vector */
void test_worker_vector_partitioned(void) {
    int arr[W][N/W];
    int wv_results[W] = {0};
    
    for (int w = 0; w < W; w++) {
        for (int i = 0; i < N/W; i++) {
            arr[w][i] = (w * 100 + i) % 100;
        }
    }
    
    #pragma acc parallel copyin(arr[0:W][0:N/W]) copyout(wv_results[0:W]) \
        num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector collapse(2)
        for (int w = 0; w < W; w++) {
            for (int i = 0; i < N/W; i++) {
                int wid = __pgi_workeridx();
                if (wid < W) {
                    wv_results[wid] += arr[wid][i];
                }
            }
        }
    }
    
    int total = 0;
    for (int w = 0; w < W; w++) total += wv_results[w];
    results[6] = total;
    global_sum += total;
}

/* Test 7: Fully partitioned - 3D with gang, worker, and vector */
void test_fully_partitioned(void) {
    int arr[G][W][V];
    int results_3d[G][W][V];
    
    /* Initialize 3D array */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr[g][w][v] = (g * 100 + w * 10 + v) % 100;
                results_3d[g][w][v] = 0;
            }
        }
    }
    
    #pragma acc parallel copyin(arr[0:G][0:W][0:V]) \
        copyout(results_3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    int gid = __pgi_gangidx();
                    int wid = __pgi_workeridx();
                    int vid = __pgi_vectoridx();
                    if (gid < G && wid < W && vid < V) {
                        results_3d[gid][wid][vid] = arr[gid][wid][vid] * 2;
                    }
                }
            }
        }
    }
    
    int total = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                total += results_3d[g][w][v];
            }
        }
    }
    results[7] = total;
    global_sum += total;
}

int main(void) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all test cases */
    test_gang_redundant();
    printf("Test 0 (gang redundant) completed: %d\n", results[0]);
    
    test_gang_partitioned();
    printf("Test 1 (gang partitioned) completed: %d\n", results[1]);
    
    test_worker_partitioned();
    printf("Test 2 (worker partitioned) completed: %d\n", results[2]);
    
    test_gang_worker_partitioned();
    printf("Test 3 (gang+worker partitioned) completed: %d\n", results[3]);
    
    test_vector_partitioned();
    printf("Test 4 (vector partitioned) completed: %d\n", results[4]);
    
    test_gang_vector_partitioned();
    printf("Test 5 (gang+vector partitioned) completed: %d\n", results[5]);
    
    test_worker_vector_partitioned();
    printf("Test 6 (worker+vector partitioned) completed: %d\n", results[6]);
    
    test_fully_partitioned();
    printf("Test 7 (fully partitioned) completed: %d\n", results[7]);
    
    /* Final checksum */
    printf("Global checksum: %d\n", global_sum);
    
    /* Verify all tests produced non-zero results */
    int all_pass = 1;
    for (int i = 0; i < 8; i++) {
        if (results[i] == 0) {
            printf("Warning: Test %d produced zero result\n", i);
            all_pass = 0;
        }
    }
    
    if (all_pass) {
        printf("All partition mapping tests completed successfully.\n");
    } else {
        printf("Some tests may have been optimized away.\n");
    }
    
    return 0;
}
