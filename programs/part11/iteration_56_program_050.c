/* Test program to cover partition mapping strings in GCC's OpenACC neuter/broadcast pass */
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
volatile int checksum = 0;

/* Test 1: Gang redundant partitioning (case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(global_1d[0:N]) copyout(local_sum) num_gangs(4)
    {
        int gang_sum = 0;
        #pragma acc loop gang reduction(+:gang_sum)
        for (int i = 0; i < N; i++) {
            gang_sum += global_1d[i];
        }
        #pragma acc single
        local_sum = gang_sum;
    }
    
    checksum += local_sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            global_1d[i] = i % 256;
        }
    }
    
    int temp = 0;
    #pragma acc parallel loop gang reduction(+:temp) copyin(global_1d[0:N])
    for (int i = 0; i < N; i++) {
        temp += global_1d[i];
    }
    checksum += temp;
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] = i * 10 + j;
            }
        }
    }
    
    int worker_sum = 0;
    #pragma acc parallel copyin(global_2d[0:W][0:V]) copy(worker_sum) num_workers(4)
    {
        int local_sum = 0;
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                local_sum += global_2d[i][j];
            }
        }
        #pragma acc single
        worker_sum = local_sum;
    }
    checksum += worker_sum;
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
        num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker collapse(2)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    global_3d[g][w][v] = g * 100 + w * 10 + v;
                }
            }
        }
    }
    
    int gw_sum = 0;
    #pragma acc parallel copyin(global_3d[0:G][0:W][0:V]) copy(gw_sum) \
        num_gangs(2) num_workers(2)
    {
        int local_sum = 0;
        #pragma acc loop gang worker reduction(+:local_sum) collapse(2)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    local_sum += global_3d[g][w][v];
                }
            }
        }
        #pragma acc single
        gw_sum = local_sum;
    }
    checksum += gw_sum;
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            global_1d[i] = i * 2;
        }
    }
    
    int vec_sum = 0;
    #pragma acc parallel loop vector reduction(+:vec_sum) copyin(global_1d[0:N])
    for (int i = 0; i < N; i++) {
        vec_sum += global_1d[i];
    }
    checksum += vec_sum;
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) \
        num_gangs(2) vector_length(32)
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] = (i + j) * 3;
            }
        }
    }
    
    int gv_sum = 0;
    #pragma acc parallel copyin(global_2d[0:W][0:V]) copy(gv_sum) \
        num_gangs(2) vector_length(32)
    {
        int local_sum = 0;
        #pragma acc loop gang vector reduction(+:local_sum) collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                local_sum += global_2d[i][j];
            }
        }
        #pragma acc single
        gv_sum = local_sum;
    }
    checksum += gv_sum;
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) \
        num_workers(2) vector_length(32)
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] = i * j + 5;
            }
        }
    }
    
    int wv_sum = 0;
    #pragma acc parallel copyin(global_2d[0:W][0:V]) copy(wv_sum) \
        num_workers(2) vector_length(32)
    {
        int local_sum = 0;
        #pragma acc loop worker vector reduction(+:local_sum) collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                local_sum += global_2d[i][j];
            }
        }
        #pragma acc single
        wv_sum = local_sum;
    }
    checksum += wv_sum;
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
        num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    global_3d[g][w][v] = g * w * v + 1;
                }
            }
        }
    }
    
    int full_sum = 0;
    #pragma acc parallel copyin(global_3d[0:G][0:W][0:V]) copy(full_sum) \
        num_gangs(2) num_workers(2) vector_length(32)
    {
        int local_sum = 0;
        #pragma acc loop gang worker vector reduction(+:local_sum) collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    local_sum += global_3d[g][w][v];
                }
            }
        }
        #pragma acc single
        full_sum = local_sum;
    }
    checksum += full_sum;
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        global_1d[i] = i % 100;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            global_2d[i][j] = 0;
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                global_3d[g][w][v] = 0;
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
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
