/* Test program to cover partition mapping strings in GCC's OpenACC neuter/broadcast pass */
#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int arr1d[N];
int arr2d[W][N/W];
int arr3d[G][W][V];

/* Test 1: Gang redundant (likely case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(arr1d[0:N]) copyout(arr1d[0:N]) reduction(+:local_sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] = i + 1;
            local_sum += arr1d[i];
        }
    }
    
    global_sum += local_sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N])
    {
        #pragma acc loop gang independent
        for (int i = 0; i < N; i++) {
            arr1d[i] *= 2;
        }
    }
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:N/W]) num_workers(W)
    {
        #pragma acc loop worker independent
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < N/W; j++) {
                arr2d[i][j] = i * j;
            }
        }
    }
}

/* Test 4: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N]) vector_length(V)
    {
        #pragma acc loop vector independent
        for (int i = 0; i < N; i++) {
            arr1d[i] += i;
        }
    }
}

/* Test 5: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G) num_workers(W)
    {
        #pragma acc loop gang worker independent collapse(2)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = g * 100 + w * 10 + v;
                }
            }
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:G][0:V]) num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang vector independent collapse(2)
        for (int g = 0; g < G; g++) {
            for (int v = 0; v < V; v++) {
                arr2d[g][v] = g * v;
            }
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector independent collapse(2)
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr2d[w][v] = w + v;
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int sum = 0;
    
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) reduction(+:sum) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector independent collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = 1;
                    sum += arr3d[g][w][v];
                }
            }
        }
    }
    
    global_sum += sum;
}

/* Additional test with complex data clause to trigger various partition mappings */
void test_complex_partitioning(void) {
    int *dynamic_arr = (int*)malloc(N * sizeof(int));
    
    #pragma acc enter data create(dynamic_arr[0:N])
    
    /* Multiple parallel regions with different partition characteristics */
    #pragma acc parallel present(dynamic_arr[0:N]) num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            dynamic_arr[i] = i % 256;
        }
    }
    
    #pragma acc parallel present(dynamic_arr[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i += 4) {
            dynamic_arr[i] *= 2;
        }
    }
    
    #pragma acc exit data copyout(dynamic_arr[0:N])
    
    /* Verify some values to prevent dead code elimination */
    int check = 0;
    for (int i = 0; i < N; i += 64) {
        check += dynamic_arr[i];
    }
    global_sum += check;
    
    free(dynamic_arr);
}

int main(void) {
    printf("Testing OpenACC partition mapping coverage...\n");
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = 0;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < N/W; j++) {
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
    test_complex_partitioning();
    
    /* Compute final checksum to ensure all computations happened */
    int final_check = global_sum;
    for (int i = 0; i < N; i++) {
        final_check += arr1d[i];
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < N/W; j++) {
            final_check += arr2d[i][j];
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                final_check += arr3d[g][w][v];
            }
        }
    }
    
    printf("Final checksum: %d\n", final_check);
    printf("If non-zero, all OpenACC regions executed.\n");
    
    return 0;
}
