/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 32 /* vector length */
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int arr1d[N];
int arr2d[W][V];
int arr3d[G][W][V];

/* Test 1: Gang redundant (case 0) - scalar reduction */
void test_gang_redundant() {
    int sum = 0;
    
    #pragma acc parallel loop gang reduction(+:sum) copyin(arr1d[0:N]) copy(sum)
    for (int i = 0; i < N; i++) {
        sum += arr1d[i];
    }
    
    global_sum += sum;
}

/* Test 2: Gang partitioned (case 1) - gang-level parallelism */
void test_gang_partitioned() {
    #pragma acc parallel loop gang copy(arr1d[0:N]) num_gangs(G)
    for (int i = 0; i < N; i++) {
        arr1d[i] = arr1d[i] * 2 + 1;
    }
}

/* Test 3: Worker partitioned (case 2) - worker-level parallelism */
void test_worker_partitioned() {
    #pragma acc parallel loop worker copy(arr2d[0:W][0:V]) num_workers(W)
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = i * V + j;
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3) - 2D decomposition */
void test_gang_worker_partitioned() {
    #pragma acc parallel loop gang worker collapse(2) copy(arr2d[0:W][0:V]) \
        num_gangs(G) num_workers(W)
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] += 1;
        }
    }
}

/* Test 5: Vector partitioned (case 4) - vector-level parallelism */
void test_vector_partitioned() {
    #pragma acc parallel loop vector copy(arr1d[0:N]) vector_length(V)
    for (int i = 0; i < N; i++) {
        arr1d[i] = arr1d[i] * 3;
    }
}

/* Test 6: Gang+vector partitioned (case 5) - gang and vector decomposition */
void test_gang_vector_partitioned() {
    #pragma acc parallel loop gang vector copy(arr1d[0:N]) \
        num_gangs(G) vector_length(V)
    for (int i = 0; i < N; i++) {
        arr1d[i] = arr1d[i] - 5;
    }
}

/* Test 7: Worker+vector partitioned (case 6) - worker and vector decomposition */
void test_worker_vector_partitioned() {
    #pragma acc parallel loop worker vector copy(arr2d[0:W][0:V]) \
        num_workers(W) vector_length(V)
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = arr2d[i][j] * 2;
        }
    }
}

/* Test 8: Fully partitioned (case 7) - 3D decomposition across all levels */
void test_fully_partitioned() {
    #pragma acc parallel loop gang worker vector collapse(3) \
        copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = i * W * V + j * V + k;
            }
        }
    }
}

/* Additional test with complex data clause to trigger partition analysis */
void test_complex_partition_mapping() {
    int partial_sums[G][W];
    
    #pragma acc data copyin(arr3d[0:G][0:W][0:V]) copyout(partial_sums[0:G][0:W])
    {
        #pragma acc parallel loop gang worker collapse(2) \
            num_gangs(G) num_workers(W) vector_length(V)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                int sum = 0;
                #pragma acc loop vector reduction(+:sum)
                for (int k = 0; k < V; k++) {
                    sum += arr3d[i][j][k];
                }
                partial_sums[i][j] = sum;
            }
        }
    }
    
    /* Use result to prevent optimization */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            global_sum += partial_sums[i][j];
        }
    }
}

/* Test with explicit data regions and update directives */
void test_data_region_partitions() {
    int local_arr[G][W][V];
    
    /* Initialize on host */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                local_arr[i][j][k] = 1;
            }
        }
    }
    
    #pragma acc data copy(local_arr[0:G][0:W][0:V])
    {
        /* Multiple parallel regions with different partitionings */
        #pragma acc parallel loop gang collapse(2) \
            num_gangs(G) num_workers(1) vector_length(1)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                int temp = 0;
                #pragma acc loop vector reduction(+:temp)
                for (int k = 0; k < V; k++) {
                    temp += local_arr[i][j][k];
                }
                local_arr[i][j][0] = temp;
            }
        }
        
        #pragma acc update self(local_arr[0:G][0:W][0:1])
    }
    
    /* Use results */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            global_sum += local_arr[i][j][0];
        }
    }
}

int main() {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i % 100;
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
    
    /* Execute tests covering different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_complex_partition_mapping();
    test_data_region_partitions();
    
    /* Verify some results to ensure execution */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += arr1d[i];
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global sum: %d\n", (int)global_sum);
    
    return 0;
}
