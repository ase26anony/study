/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 * Or for AMD: gcc -O2 -fopenacc -foffload=amdgcn-amdhsa -o test_partitions test_partitions.c
 */

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
    
    #pragma acc parallel copyin(arr1d[0:N]) copyout(arr1d[0:N]) reduction(+:local_sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] = i % 100;
            local_sum += arr1d[i];
        }
    }
    
    global_sum += local_sum;
}

/* Test gang partitioned (case 1) */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = g * 100 + w * 10 + v;
                }
            }
        }
    }
}

/* Test worker partitioned (case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector independent
            for (int v = 0; v < V; v++) {
                arr2d[w][v] = w * 10 + v;
            }
        }
    }
}

/* Test gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int sum = 0;
    
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) reduction(+:sum) \
                num_gangs(G) num_workers(W)
    {
        #pragma acc loop gang worker independent collapse(2)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] += 1;
                    sum += arr3d[g][w][v];
                }
            }
        }
    }
    
    global_sum += sum;
}

/* Test vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N]) vector_length(V)
    {
        #pragma acc loop vector independent
        for (int i = 0; i < N; i++) {
            arr1d[i] = arr1d[i] * 2;
        }
    }
}

/* Test gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang vector independent collapse(2)
        for (int g = 0; g < G; g++) {
            for (int v = 0; v < V; v++) {
                #pragma acc loop worker independent
                for (int w = 0; w < W; w++) {
                    arr3d[g][w][v] = arr3d[g][w][v] * 3;
                }
            }
        }
    }
}

/* Test worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector independent collapse(2)
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr2d[w][v] = arr2d[w][v] + 5;
            }
        }
    }
}

/* Test fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int total = 0;
    
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) reduction(+:total) \
                num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector independent collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = arr3d[g][w][v] * 2 + 1;
                    total += arr3d[g][w][v];
                }
            }
        }
    }
    
    global_sum += total;
}

/* Additional test with complex data clauses */
void test_complex_partitioning(void) {
    int partial[G][W];
    
    #pragma acc data create(partial[0:G][0:W]) copy(arr3d[0:G][0:W][0:V])
    {
        #pragma acc parallel num_gangs(G) num_workers(W) vector_length(V)
        {
            #pragma acc loop gang worker independent collapse(2)
            for (int g = 0; g < G; g++) {
                for (int w = 0; w < W; w++) {
                    int sum = 0;
                    #pragma acc loop vector reduction(+:sum)
                    for (int v = 0; v < V; v++) {
                        sum += arr3d[g][w][v];
                    }
                    partial[g][w] = sum;
                }
            }
        }
        
        #pragma acc parallel num_gangs(G)
        {
            #pragma acc loop gang independent
            for (int g = 0; g < G; g++) {
                int gang_sum = 0;
                #pragma acc loop worker reduction(+:gang_sum)
                for (int w = 0; w < W; w++) {
                    gang_sum += partial[g][w];
                }
                arr3d[g][0][0] = gang_sum;
            }
        }
    }
}

int main(void) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i;
    }
    
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr2d[w][v] = w * V + v;
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr3d[g][w][v] = g * W * V + w * V + v;
            }
        }
    }
    
    /* Execute tests covering all partition types */
    test_gang_redundant();          /* Should trigger case 0 */
    test_gang_partitioned();        /* Should trigger case 1 */
    test_worker_partitioned();      /* Should trigger case 2 */
    test_gang_worker_partitioned(); /* Should trigger case 3 */
    test_vector_partitioned();      /* Should trigger case 4 */
    test_gang_vector_partitioned(); /* Should trigger case 5 */
    test_worker_vector_partitioned(); /* Should trigger case 6 */
    test_fully_partitioned();       /* Should trigger case 7 */
    test_complex_partitioning();    /* Mixed partitioning */
    
    /* Compute verification checksum */
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
    printf("If checksum is non-zero, all OpenACC regions executed.\n");
    
    return 0;
}
