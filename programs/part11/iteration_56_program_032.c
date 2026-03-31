/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
int global_3d[G][W][V];
int global_2d[W][V];
int global_1d[N];
volatile int volatile_sum = 0;

/* Test 1: Gang redundant partitioning - likely case 0 */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(global_1d[0:N]) copyout(local_sum) num_gangs(4)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            local_sum += global_1d[i];
        }
    }
    
    volatile_sum += local_sum;
}

/* Test 2: Gang partitioned - case 1 */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            global_1d[i] = i * 2;
        }
    }
    
    /* Force use of result */
    volatile_sum += global_1d[N/2];
}

/* Test 3: Worker partitioned - case 2 */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            #pragma acc loop vector
            for (int j = 0; j < V; j++) {
                global_2d[i][j] = i * 10 + j;
            }
        }
    }
    
    /* Force use of result */
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            volatile_sum += global_2d[i][j];
        }
    }
}

/* Test 4: Gang+worker partitioned - case 3 */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector
                for (int v = 0; v < V; v++) {
                    global_3d[g][w][v] = g * 100 + w * 10 + v;
                }
            }
        }
    }
    
    /* Force use of result */
    volatile_sum += global_3d[0][0][0];
}

/* Test 5: Vector partitioned - case 4 */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) vector_length(128)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            global_1d[i] += 1;
        }
    }
    
    /* Force use of result */
    volatile_sum += global_1d[0];
}

/* Test 6: Gang+vector partitioned - case 5 */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) num_gangs(2) vector_length(64)
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] *= 2;
            }
        }
    }
    
    /* Force use of result */
    volatile_sum += global_2d[W-1][V-1];
}

/* Test 7: Worker+vector partitioned - case 6 */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) num_workers(2) vector_length(64)
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] += i + j;
            }
        }
    }
    
    /* Force use of result */
    volatile_sum += global_2d[0][0];
}

/* Test 8: Fully partitioned - case 7 */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
        num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    global_3d[g][w][v] = (g + w + v) % 256;
                }
            }
        }
    }
    
    /* Force use of result */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                volatile_sum += global_3d[g][w][v];
            }
        }
    }
}

/* Test 9: Illegal partition code - default case */
/* This might be harder to trigger directly, but we include various
 * unusual configurations that might lead to edge cases */
void test_edge_cases(void) {
    /* Mixed directives that might confuse partitioning */
    #pragma acc parallel copy(global_1d[0:16]) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop seq  /* Sequential loop - unusual in parallel region */
        for (int i = 0; i < 16; i++) {
            global_1d[i] = i;
        }
    }
    
    volatile_sum += global_1d[0];
    
    /* Empty parallel region */
    #pragma acc parallel num_gangs(1)
    {
        /* No operations - might trigger special handling */
    }
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        global_1d[i] = i % 100;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            global_2d[i][j] = (i * V + j) % 50;
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                global_3d[g][w][v] = (g * W * V + w * V + v) % 200;
            }
        }
    }
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    printf("  Test 1 (gang redundant) completed\n");
    
    test_gang_partitioned();
    printf("  Test 2 (gang partitioned) completed\n");
    
    test_worker_partitioned();
    printf("  Test 3 (worker partitioned) completed\n");
    
    test_gang_worker_partitioned();
    printf("  Test 4 (gang+worker partitioned) completed\n");
    
    test_vector_partitioned();
    printf("  Test 5 (vector partitioned) completed\n");
    
    test_gang_vector_partitioned();
    printf("  Test 6 (gang+vector partitioned) completed\n");
    
    test_worker_vector_partitioned();
    printf("  Test 7 (worker+vector partitioned) completed\n");
    
    test_fully_partitioned();
    printf("  Test 8 (fully partitioned) completed\n");
    
    test_edge_cases();
    printf("  Test 9 (edge cases) completed\n");
    
    /* Print checksum to ensure all computations happened */
    printf("\nFinal checksum: %d\n", volatile_sum);
    printf("All tests completed successfully.\n");
    
    return 0;
}
