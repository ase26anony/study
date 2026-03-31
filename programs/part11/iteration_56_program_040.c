/* Test program to cover partition mapping strings in GCC's OpenACC neuter/broadcast pass */
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

/* Test 1: Gang redundant partition (likely case 0) */
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
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(W)
    {
        #pragma acc loop gang independent
        for (int i = 0; i < W; i++) {
            #pragma acc loop vector
            for (int j = 0; j < V; j++) {
                arr2d[i][j] = i * 100 + j;
            }
        }
    }
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            #pragma acc loop vector
            for (int j = 0; j < V; j++) {
                arr2d[i][j] += 1;
            }
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = g * 1000 + w * 100 + v;
                }
            }
        }
    }
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N]) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr1d[i] += i % 7;
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                for (int w = 0; w < W; w++) {
                    arr3d[g][w][v] += g + w + v;
                }
            }
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) \
        num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                arr2d[w][v] *= 2;
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = (arr3d[g][w][v] + 1) % 256;
                }
            }
        }
    }
}

/* Test 9: Mixed partitioning with data clauses */
void test_mixed_partitioning(void) {
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
    }
    
    /* Complex data clause with multiple arrays */
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(32) num_workers(2) vector_length(64)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2;
        }
    }
    
    /* Verify */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    global_sum += sum;
}

int main(void) {
    printf("Testing OpenACC partition mapping coverage...\n");
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = i * 10 + j;
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr3d[g][w][v] = g * 100 + w * 10 + v;
            }
        }
    }
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_mixed_partitioning();
    
    /* Compute checksum to ensure all computations happened */
    int checksum = global_sum;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                checksum += arr3d[g][w][v];
            }
        }
    }
    
    for (int i = 0; i < N; i++) {
        checksum += arr1d[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum != 0, computations were performed.\n");
    
    return 0;
}
