/* Test program to cover partition mapping strings in GCC's OpenACC neuter/broadcast */
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

/* Test 1: Gang redundant/partitioned */
void test_gang_redundant(void) {
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
    printf("Gang test completed, sum = %d\n", local_sum);
}

/* Test 2: Worker partitioned */
void test_worker_partitioned(void) {
    int worker_sum = 0;
    
    #pragma acc parallel copy(arr2d[0:W][0:V]) reduction(+:worker_sum)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                arr2d[i][j] = (i * 10) + j;
                worker_sum += arr2d[i][j];
            }
        }
    }
    
    global_sum += worker_sum;
    printf("Worker test completed, sum = %d\n", worker_sum);
}

/* Test 3: Vector partitioned */
void test_vector_partitioned(void) {
    int vector_sum = 0;
    
    #pragma acc parallel copy(arr1d[0:N]) reduction(+:vector_sum)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr1d[i] += i;
            vector_sum += arr1d[i];
        }
    }
    
    global_sum += vector_sum;
    printf("Vector test completed, sum = %d\n", vector_sum);
}

/* Test 4: Gang+worker partitioned */
void test_gang_worker_partitioned(void) {
    int gw_sum = 0;
    
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) reduction(+:gw_sum) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    arr3d[i][j][k] = (i * 100) + (j * 10) + k;
                    gw_sum += arr3d[i][j][k];
                }
            }
        }
    }
    
    global_sum += gw_sum;
    printf("Gang+Worker test completed, sum = %d\n", gw_sum);
}

/* Test 5: Gang+vector partitioned */
void test_gang_vector_partitioned(void) {
    int gv_sum = 0;
    
    #pragma acc parallel copy(arr2d[0:W][0:V]) reduction(+:gv_sum) \
        num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                arr2d[i][j] *= 2;
                gv_sum += arr2d[i][j];
            }
        }
    }
    
    global_sum += gv_sum;
    printf("Gang+Vector test completed, sum = %d\n", gv_sum);
}

/* Test 6: Worker+vector partitioned */
void test_worker_vector_partitioned(void) {
    int wv_sum = 0;
    
    #pragma acc parallel copy(arr2d[0:W][0:V]) reduction(+:wv_sum) \
        num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                arr2d[i][j] += 1;
                wv_sum += arr2d[i][j];
            }
        }
    }
    
    global_sum += wv_sum;
    printf("Worker+Vector test completed, sum = %d\n", wv_sum);
}

/* Test 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(void) {
    int full_sum = 0;
    
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) reduction(+:full_sum) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    arr3d[i][j][k] += (i + j + k);
                    full_sum += arr3d[i][j][k];
                }
            }
        }
    }
    
    global_sum += full_sum;
    printf("Fully partitioned test completed, sum = %d\n", full_sum);
}

/* Test 8: Mixed partitioning with data clauses */
void test_mixed_partitioning(void) {
    int mixed_sum = 0;
    int local_arr[G][W];
    
    /* Create with explicit shape to trigger partition analysis */
    #pragma acc data create(local_arr[0:G][0:W])
    {
        #pragma acc parallel present(local_arr) reduction(+:mixed_sum) \
            num_gangs(G) num_workers(W)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < G; i++) {
                for (int j = 0; j < W; j++) {
                    local_arr[i][j] = i * j;
                    mixed_sum += local_arr[i][j];
                }
            }
        }
        
        #pragma acc parallel present(local_arr) reduction(+:mixed_sum) \
            vector_length(V)
        {
            #pragma acc loop vector
            for (int i = 0; i < G; i++) {
                for (int j = 0; j < W; j++) {
                    local_arr[i][j] += 5;
                    mixed_sum += local_arr[i][j];
                }
            }
        }
    }
    
    global_sum += mixed_sum;
    printf("Mixed partitioning test completed, sum = %d\n", mixed_sum);
}

int main(void) {
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = 0;
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = 0;
            }
        }
    }
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();           /* Should trigger case 0 or 1 */
    test_worker_partitioned();       /* Should trigger case 2 */
    test_vector_partitioned();       /* Should trigger case 4 */
    test_gang_worker_partitioned();  /* Should trigger case 3 */
    test_gang_vector_partitioned();  /* Should trigger case 5 */
    test_worker_vector_partitioned();/* Should trigger case 6 */
    test_fully_partitioned();        /* Should trigger case 7 */
    test_mixed_partitioning();       /* Additional coverage */
    
    printf("\nAll tests completed. Global sum = %d\n", global_sum);
    printf("(Note: The actual partition mapping strings are internal to GCC)\n");
    
    return 0;
}
