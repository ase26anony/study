/* Test program to cover partition mapping strings in GCC's OpenACC neuter/broadcast pass.
   Designed to trigger all cases (0-7) of the partition code to string mapping function. */

#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs dimension */
#define W 4  /* workers dimension */
#define V 2  /* vectors dimension */
#define N 1024

/* Global arrays to store results and prevent optimization */
volatile int global_sum = 0;
int results[8] = {0};  /* One result per test case */
int arr3d[G][W][V];
int arr2d[W][V];
int arr1d[N];

/* Initialize arrays with test data */
void init_arrays() {
    for (int i = 0; i < N; i++) arr1d[i] = i % 100;
    for (int i = 0; i < W; i++)
        for (int j = 0; j < V; j++)
            arr2d[i][j] = i * 10 + j;
    for (int i = 0; i < G; i++)
        for (int j = 0; j < W; j++)
            for (int k = 0; k < V; k++)
                arr3d[i][j][k] = i * 100 + j * 10 + k;
}

/* Test 1: Gang redundant (likely case 0) - scalar reduction */
void test_gang_redundant() {
    int sum = 0;
    #pragma acc parallel copyin(arr1d[0:N]) copy(sum) num_gangs(4)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += arr1d[i] % 17;  /* Non-trivial computation */
        }
    }
    results[0] = sum;
    global_sum += sum;
}

/* Test 2: Gang partitioned (case 1) - gang-level parallelism */
void test_gang_partitioned() {
    int partial[G] = {0};
    #pragma acc parallel copyin(arr3d[0:G][0:W][0:V]) copyout(partial[0:G]) \
        num_gangs(G) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            int local = 0;
            #pragma acc loop worker vector collapse(2)
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    local += arr3d[g][w][v];
                }
            }
            partial[g] = local;
        }
    }
    for (int i = 0; i < G; i++) results[1] += partial[i];
    global_sum += results[1];
}

/* Test 3: Worker partitioned (case 2) - worker-level parallelism */
void test_worker_partitioned() {
    int worker_results[W] = {0};
    #pragma acc parallel copyin(arr2d[0:W][0:V]) copyout(worker_results[0:W]) \
        num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            int sum = 0;
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                sum += arr2d[w][v];
            }
            worker_results[w] = sum;
        }
    }
    for (int i = 0; i < W; i++) results[2] += worker_results[i];
    global_sum += results[2];
}

/* Test 4: Gang+worker partitioned (case 3) - 2D decomposition */
void test_gang_worker_partitioned() {
    int gw_results[G][W];
    #pragma acc parallel copyin(arr3d[0:G][0:W][0:V]) copyout(gw_results[0:G][0:W]) \
        num_gangs(G) num_workers(W) vector_length(1)
    {
        #pragma acc loop gang worker
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                int sum = 0;
                #pragma acc loop vector
                for (int v = 0; v < V; v++) {
                    sum += arr3d[g][w][v] % 13;
                }
                gw_results[g][w] = sum;
            }
        }
    }
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            results[3] += gw_results[g][w];
    global_sum += results[3];
}

/* Test 5: Vector partitioned (case 4) - vector-level parallelism */
void test_vector_partitioned() {
    int vec_results[V] = {0};
    #pragma acc parallel copyin(arr2d[0:1][0:V]) copyout(vec_results[0:V]) \
        num_gangs(1) num_workers(1) vector_length(V)
    {
        #pragma acc loop vector
        for (int v = 0; v < V; v++) {
            vec_results[v] = arr2d[0][v] * 3;
        }
    }
    for (int i = 0; i < V; i++) results[4] += vec_results[i];
    global_sum += results[4];
}

/* Test 6: Gang+vector partitioned (case 5) - gang and vector decomposition */
void test_gang_vector_partitioned() {
    int gv_results[G][V];
    #pragma acc parallel copyin(arr3d[0:G][0:1][0:V]) copyout(gv_results[0:G][0:V]) \
        num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang vector
        for (int g = 0; g < G; g++) {
            for (int v = 0; v < V; v++) {
                gv_results[g][v] = arr3d[g][0][v] * 2;
            }
        }
    }
    for (int g = 0; g < G; g++)
        for (int v = 0; v < V; v++)
            results[5] += gv_results[g][v];
    global_sum += results[5];
}

/* Test 7: Worker+vector partitioned (case 6) - worker and vector decomposition */
void test_worker_vector_partitioned() {
    int wv_results[W][V];
    #pragma acc parallel copyin(arr2d[0:W][0:V]) copyout(wv_results[0:W][0:V]) \
        num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                wv_results[w][v] = arr2d[w][v] + 5;
            }
        }
    }
    for (int w = 0; w < W; w++)
        for (int v = 0; v < V; v++)
            results[6] += wv_results[w][v];
    global_sum += results[6];
}

/* Test 8: Fully partitioned (case 7) - 3D decomposition across all levels */
void test_fully_partitioned() {
    int f_results[G][W][V];
    #pragma acc parallel copyin(arr3d[0:G][0:W][0:V]) copyout(f_results[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    f_results[g][w][v] = arr3d[g][w][v] * 2 - 1;
                }
            }
        }
    }
    for (int g = 0; g < G; g++)
        for (int w = 0; w < W; w++)
            for (int v = 0; v < V; v++)
                results[7] += f_results[g][w][v];
    global_sum += results[7];
}

int main() {
    init_arrays();
    
    printf("Running OpenACC partition mapping tests...\n");
    
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Print results to prevent optimization */
    printf("Results: ");
    for (int i = 0; i < 8; i++) {
        printf("%d ", results[i]);
    }
    printf("\nGlobal checksum: %d\n", global_sum);
    
    return 0;
}
