/* Test program to cover partition mapping cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int arr3d[G][W][V];
int arr2d[W][V];
int arr1d[N];

/* Test 1: Gang redundant partitioning (case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(arr1d[0:N]) copyout(arr1d[0:N]) reduction(+:local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            arr1d[i] = i * 2;
            local_sum += arr1d[i];
        }
    }
    
    global_sum += local_sum;
    printf("Gang redundant test complete, sum = %d\n", local_sum);
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = g * 100 + w * 10 + v;
                }
            }
        }
    }
    
    /* Verify some values */
    int check = arr3d[1][2][1] + arr3d[3][0][0];
    global_sum += check;
    printf("Gang partitioned test complete, check = %d\n", check);
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector independent
            for (int v = 0; v < V; v++) {
                arr2d[w][v] = w * 10 + v + 1000;
            }
        }
    }
    
    int check = 0;
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            check += arr2d[w][v];
        }
    }
    global_sum += check;
    printf("Worker partitioned test complete, check = %d\n", check);
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int partial_sums[G][W] = {{0}};
    
    #pragma acc parallel copyin(arr3d[0:G][0:W][0:V]) copyout(partial_sums[0:G][0:W]) \
        num_gangs(G) num_workers(W)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < W; w++) {
                int sum = 0;
                #pragma acc loop vector reduction(+:sum)
                for (int v = 0; v < V; v++) {
                    sum += arr3d[g][w][v];
                }
                partial_sums[g][w] = sum;
            }
        }
    }
    
    int total = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            total += partial_sums[g][w];
        }
    }
    global_sum += total;
    printf("Gang+worker partitioned test complete, total = %d\n", total);
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N]) vector_length(V)
    {
        #pragma acc loop vector independent
        for (int i = 0; i < N; i++) {
            arr1d[i] = arr1d[i] * 3 + 1;
        }
    }
    
    int check = arr1d[0] + arr1d[N-1];
    global_sum += check;
    printf("Vector partitioned test complete, check = %d\n", check);
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int results[G][V] = {{0}};
    
    #pragma acc parallel copyin(arr3d[0:G][0:1][0:V]) copyout(results[0:G][0:V]) \
        num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop vector independent
            for (int v = 0; v < V; v++) {
                results[g][v] = arr3d[g][0][v] * 2;
            }
        }
    }
    
    int total = 0;
    for (int g = 0; g < G; g++) {
        for (int v = 0; v < V; v++) {
            total += results[g][v];
        }
    }
    global_sum += total;
    printf("Gang+vector partitioned test complete, total = %d\n", total);
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int transformed[W][V];
    
    #pragma acc parallel copyin(arr2d[0:W][0:V]) copyout(transformed[0:W][0:V]) \
        num_workers(W) vector_length(V)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector independent
            for (int v = 0; v < V; v++) {
                transformed[w][v] = arr2d[w][v] + 500;
            }
        }
    }
    
    int check = 0;
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            check += transformed[w][v];
        }
    }
    global_sum += check;
    printf("Worker+vector partitioned test complete, check = %d\n", check);
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int final_result = 0;
    
    #pragma acc parallel copyin(arr3d[0:G][0:W][0:V]) copy(final_result) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector reduction(+:final_result) collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    final_result += arr3d[g][w][v];
                }
            }
        }
    }
    
    global_sum += final_result;
    printf("Fully partitioned test complete, result = %d\n", final_result);
}

/* Test 9: Mixed partitioning patterns */
void test_mixed_patterns(void) {
    /* This test uses multiple parallel regions with different partitionings */
    
    /* First region: gang redundant */
    int temp = 0;
    #pragma acc parallel copy(temp) reduction(+:temp)
    {
        #pragma acc loop gang
        for (int i = 0; i < 10; i++) {
            temp += i;
        }
    }
    
    /* Second region: worker partitioned */
    int temp2[W] = {0};
    #pragma acc parallel copyin(arr2d[0:W][0:1]) copyout(temp2[0:W]) num_workers(W)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            temp2[w] = arr2d[w][0] * 2;
        }
    }
    
    /* Third region: vector partitioned */
    #pragma acc parallel copy(arr1d[0:V]) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < V; i++) {
            arr1d[i] += 100;
        }
    }
    
    int mixed_check = temp;
    for (int w = 0; w < W; w++) {
        mixed_check += temp2[w];
    }
    mixed_check += arr1d[0];
    
    global_sum += mixed_check;
    printf("Mixed patterns test complete, check = %d\n", mixed_check);
}

int main(void) {
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i;
    }
    
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr2d[w][v] = w * 100 + v;
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr3d[g][w][v] = g * 1000 + w * 100 + v * 10;
            }
        }
    }
    
    /* Run all tests to trigger different partition mappings */
    test_gang_redundant();           /* Should trigger case 0 */
    test_gang_partitioned();         /* Should trigger case 1 */
    test_worker_partitioned();       /* Should trigger case 2 */
    test_gang_worker_partitioned();  /* Should trigger case 3 */
    test_vector_partitioned();       /* Should trigger case 4 */
    test_gang_vector_partitioned();  /* Should trigger case 5 */
    test_worker_vector_partitioned();/* Should trigger case 6 */
    test_fully_partitioned();        /* Should trigger case 7 */
    test_mixed_patterns();           /* Should trigger multiple cases */
    
    /* Also test with different data clause combinations */
    int extra_arr[G][V];
    #pragma acc parallel copy(extra_arr[0:G][0:V]) num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang vector independent collapse(2)
        for (int g = 0; g < G; g++) {
            for (int v = 0; v < V; v++) {
                extra_arr[g][v] = g * v;
            }
        }
    }
    
    printf("\nAll tests completed. Global sum = %d\n", global_sum);
    printf("If compiled with OpenACC offload, this should exercise partition mapping codes 0-7.\n");
    
    return 0;
}
