/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: case 0-7 return strings for different partition types
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 * Or with AMD: gcc -O2 -fopenacc -foffload=amdgcn-amdhsa -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
int arr3d[G][W][V];
int arr2d[W][V];
int arr1d[N];
int results[8] = {0}; /* Store results from each test */

/* Test 1: Gang redundant (likely case 0) */
void test_gang_redundant(void) {
    int sum = 0;
    #pragma acc parallel copyin(arr1d[0:N]) copy(sum) num_gangs(4)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += arr1d[i];
        }
    }
    results[0] = sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] += g + 1; /* Each gang adds its index */
                }
            }
        }
    }
    results[1] = arr3d[0][0][0];
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr2d[w][v] += w * 10 + v; /* Each worker processes a row */
            }
        }
    }
    results[2] = arr2d[0][0];
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G) num_workers(W)
    {
        #pragma acc loop gang worker collapse(2)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] += (g * 100) + (w * 10) + v;
                }
            }
        }
    }
    results[3] = arr3d[G-1][W-1][V-1];
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N]) vector_length(V*4)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr1d[i] += i % 16; /* Vector operations */
        }
    }
    results[4] = arr1d[N-1];
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang vector collapse(2)
        for (int g = 0; g < G; g++) {
            for (int v = 0; v < V; v++) {
                for (int w = 0; w < W; w++) {
                    arr3d[g][w][v] += (g * 64) + (v * 8) + w;
                }
            }
        }
    }
    results[5] = arr3d[0][0][0];
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr2d[w][v] += (w * V) + v + 1000;
            }
        }
    }
    results[6] = arr2d[W-1][V-1];
}

/* Test 8: Fully partitioned (case 7) - gang+worker+vector */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = (g * 1000) + (w * 100) + (v * 10) + 1;
                }
            }
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    #pragma acc parallel copyin(arr3d[0:G][0:W][0:V]) copy(checksum) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector reduction(+:checksum) collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    checksum += arr3d[g][w][v];
                }
            }
        }
    }
    results[7] = checksum;
}

int main(void) {
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i % 256;
    }
    
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr2d[w][v] = w * 10 + v;
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr3d[g][w][v] = g * 100 + w * 10 + v;
            }
        }
    }
    
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
    
    /* Print results to ensure computation happened */
    printf("Results: ");
    for (int i = 0; i < 8; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    /* Final checksum verification */
    int final_check = 0;
    for (int i = 0; i < 8; i++) {
        final_check += results[i];
    }
    printf("Final checksum: %d\n", final_check);
    
    return 0;
}
