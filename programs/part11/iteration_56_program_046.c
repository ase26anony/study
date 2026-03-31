/* Test program to cover partition mapping in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 2  /* vectors */
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int output_array[N];
int array_3d[G][W][V];
int array_2d[W][V];
int array_1d[N];

/* Initialize arrays */
void init_arrays() {
    for (int i = 0; i < N; i++) {
        array_1d[i] = i % 100;
        output_array[i] = 0;
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                array_3d[g][w][v] = g * 100 + w * 10 + v;
            }
        }
    }
    
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            array_2d[w][v] = w * 10 + v;
        }
    }
}

/* Test 1: Gang redundant partitioning (case 0) */
void test_gang_redundant() {
    int sum = 0;
    
    #pragma acc parallel copyin(array_1d[0:N]) copyout(output_array[0:N]) \
                         reduction(+:sum) num_gangs(4)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            output_array[i] = array_1d[i] + 1;
            sum += array_1d[i];
        }
    }
    
    global_sum += sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned() {
    #pragma acc parallel copy(array_1d[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            array_1d[i] *= 2;
        }
    }
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned() {
    int local_sum = 0;
    
    #pragma acc parallel copyin(array_2d[0:W][0:V]) copyout(array_2d[0:W][0:V]) \
                         num_workers(W) vector_length(V)
    {
        #pragma acc loop worker reduction(+:local_sum)
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                array_2d[w][v] += w + v;
                local_sum += array_2d[w][v];
            }
        }
    }
    
    global_sum += local_sum;
}

/* Test 4: Vector partitioned (case 4) */
void test_vector_partitioned() {
    #pragma acc parallel copy(array_1d[0:N]) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            array_1d[i] = array_1d[i] * 3 + i;
        }
    }
}

/* Test 5: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned() {
    int result[G][W];
    
    #pragma acc parallel create(result[0:G][0:W]) \
                         copyin(array_3d[0:G][0:W][0:V]) \
                         num_gangs(G) num_workers(W)
    {
        #pragma acc loop gang worker
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                int sum = 0;
                #pragma acc loop vector reduction(+:sum)
                for (int v = 0; v < V; v++) {
                    sum += array_3d[g][w][v];
                }
                result[g][w] = sum;
            }
        }
    }
    
    /* Use result to prevent optimization */
    int check = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            check += result[g][w];
        }
    }
    global_sum += check;
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned() {
    #pragma acc parallel copy(array_1d[0:N]) num_gangs(4) vector_length(32)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            array_1d[i] = (array_1d[i] + i) % 256;
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned() {
    int temp[W][V];
    
    #pragma acc parallel copyin(array_2d[0:W][0:V]) copyout(temp[0:W][0:V]) \
                         num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                temp[w][v] = array_2d[w][v] * 2 + v;
            }
        }
    }
    
    int check = 0;
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            check += temp[w][v];
        }
    }
    global_sum += check;
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned() {
    int result[G][W][V];
    
    #pragma acc parallel copyin(array_3d[0:G][0:W][0:V]) \
                         copyout(result[0:G][0:W][0:V]) \
                         num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    result[g][w][v] = array_3d[g][w][v] * 2 + g + w + v;
                }
            }
        }
    }
    
    int check = 0;
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                check += result[g][w][v];
            }
        }
    }
    global_sum += check;
}

int main() {
    printf("Testing OpenACC partition mappings...\n");
    
    init_arrays();
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_vector_partitioned();
    test_gang_worker_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Additional mixed test to cover edge cases */
    #pragma acc parallel copy(array_1d[0:N]) num_gangs(2) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            array_1d[i] = array_1d[i] + 1;
        }
    }
    
    /* Verify results */
    int final_check = 0;
    for (int i = 0; i < N; i++) {
        final_check += array_1d[i];
    }
    final_check += global_sum;
    
    printf("Final checksum: %d\n", final_check);
    printf("Test completed.\n");
    
    return 0;
}
