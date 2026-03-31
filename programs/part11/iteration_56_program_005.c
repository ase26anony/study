/* Test program to cover partition mapping in omp-oacc-neuter-broadcast.cc */
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

/* Test gang redundant partitioning (case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(arr1d[0:N]) copyout(local_sum) num_gangs(4)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            local_sum += arr1d[i];
        }
    }
    
    global_sum += local_sum;
}

/* Test gang partitioned (case 1) */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] = arr1d[i] * 2 + 1;
        }
    }
}

/* Test worker partitioned (case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                arr2d[i][j] = i * 10 + j;
            }
        }
    }
}

/* Test gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    arr3d[i][j][k] = (i * 100) + (j * 10) + k;
                }
            }
        }
    }
}

/* Test vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N]) vector_length(64)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr1d[i] = arr1d[i] + (i & 0xFF);
        }
    }
}

/* Test gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(2) vector_length(32)
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                arr2d[i][j] = arr2d[i][j] * 3 - 5;
            }
        }
    }
}

/* Test worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(2) vector_length(32)
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                arr2d[i][j] = arr2d[i][j] / 2 + 7;
            }
        }
    }
}

/* Test fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int checksum = 0;
    
    #pragma acc parallel copyin(arr3d[0:G][0:W][0:V]) copyout(checksum) \
                num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:checksum) collapse(3)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    checksum += arr3d[i][j][k];
                }
            }
        }
    }
    
    global_sum += checksum;
}

/* Additional test with complex data clauses */
void test_mixed_partitions(void) {
    int temp[G][W];
    
    /* Create with explicit dimensions */
    #pragma acc data create(temp[0:G][0:W])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < G; i++) {
                #pragma acc loop vector
                for (int j = 0; j < W; j++) {
                    temp[i][j] = i * j;
                }
            }
        }
        
        /* Copy out partial results */
        #pragma acc parallel copyout(temp[2:4][0:W]) num_gangs(2)
        {
            #pragma acc loop gang
            for (int i = 2; i < 6; i++) {
                for (int j = 0; j < W; j++) {
                    temp[i][j] = temp[i][j] * 2;
                }
            }
        }
    }
}

int main(void) {
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
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_mixed_partitions();
    
    /* Compute final checksum to ensure all computations happened */
    int final_checksum = global_sum;
    for (int i = 0; i < N; i++) {
        final_checksum += arr1d[i];
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            final_checksum += arr2d[i][j];
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                final_checksum += arr3d[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Tests completed.\n");
    
    return 0;
}
