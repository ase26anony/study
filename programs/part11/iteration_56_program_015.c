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

/* Initialize arrays */
void init_arrays() {
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr3d[g][w][v] = g * 100 + w * 10 + v;
            }
        }
    }
    
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr2d[w][v] = w * 10 + v;
        }
    }
    
    for (int i = 0; i < N; i++) {
        arr1d[i] = i;
    }
}

/* Test 1: Gang redundant - likely case 0 */
void test_gang_redundant() {
    int local_sum = 0;
    
    #pragma acc parallel copyin(arr1d[0:N]) copy(local_sum) num_gangs(4)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            local_sum += arr1d[i];
        }
    }
    
    global_sum += local_sum;
}

/* Test 2: Gang partitioned - likely case 1 */
void test_gang_partitioned() {
    #pragma acc parallel copy(arr1d[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] += 1;
        }
    }
}

/* Test 3: Worker partitioned - likely case 2 */
void test_worker_partitioned() {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(2) vector_length(32)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr2d[w][v] *= 2;
            }
        }
    }
}

/* Test 4: Gang+worker partitioned - likely case 3 */
void test_gang_worker_partitioned() {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker collapse(2)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] += g + w + v;
                }
            }
        }
    }
}

/* Test 5: Vector partitioned - likely case 4 */
void test_vector_partitioned() {
    #pragma acc parallel copy(arr1d[0:N]) vector_length(64)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr1d[i] = arr1d[i] * 3;
        }
    }
}

/* Test 6: Gang+vector partitioned - likely case 5 */
void test_gang_vector_partitioned() {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(2) vector_length(32)
    {
        #pragma acc loop gang vector collapse(2)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] -= v;
                }
            }
        }
    }
}

/* Test 7: Worker+vector partitioned - likely case 6 */
void test_worker_vector_partitioned() {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(2) vector_length(32)
    {
        #pragma acc loop worker vector
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr2d[w][v] += w * v;
            }
        }
    }
}

/* Test 8: Fully partitioned - likely case 7 */
void test_fully_partitioned() {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = (arr3d[g][w][v] % 17) + 1;
                }
            }
        }
    }
}

/* Test 9: Mixed partitioning with data clauses */
void test_mixed_partitioning() {
    int temp[G][W];
    
    #pragma acc data copyin(arr3d) create(temp) copyout(arr2d)
    {
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang worker
            for (int g = 0; g < G; g++) {
                for (int w = 0; w < W; w++) {
                    int sum = 0;
                    #pragma acc loop vector reduction(+:sum)
                    for (int v = 0; v < V; v++) {
                        sum += arr3d[g][w][v];
                    }
                    temp[g][w] = sum;
                }
            }
            
            #pragma acc loop worker
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr2d[w][v] = 0;
                    #pragma acc loop gang
                    for (int g = 0; g < G; g++) {
                        arr2d[w][v] += temp[g][w];
                    }
                }
            }
        }
    }
}

int main() {
    printf("Starting OpenACC partition coverage test...\n");
    
    init_arrays();
    
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
    
    /* Compute checksum to ensure all computations executed */
    int checksum = global_sum;
    for (int i = 0; i < N; i++) {
        checksum += arr1d[i];
    }
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            checksum += arr2d[w][v];
        }
    }
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                checksum += arr3d[g][w][v];
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
