/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc
   Lines 335-343: case 0-7 return strings for different partition types
   Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
   Or for AMD: gcc -O2 -fopenacc -foffload=amdgcn-amdhsa -o test_partitions test_partitions.c
*/

#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 32 /* vector length */
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int arr3d[G][W][V];
int arr2d[W][V];
int arr1d[N];

/* Test 1: Gang redundant (case 0) - scalar reduction */
void test_gang_redundant() {
    int local_sum = 0;
    
    #pragma acc parallel copyin(arr1d[0:N]) copy(local_sum) num_gangs(G)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            local_sum += arr1d[i];
        }
    }
    
    global_sum += local_sum;
}

/* Test 2: Gang partitioned (case 1) - each gang works on different data section */
void test_gang_partitioned() {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(G) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            /* Each gang processes different workers */
            int w = g % W;
            #pragma acc loop worker
            for (int v = 0; v < V; v++) {
                arr2d[w][v] += g + 1;
            }
        }
    }
}

/* Test 3: Worker partitioned (case 2) - explicit worker parallelism */
void test_worker_partitioned() {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(1) num_workers(W) vector_length(V)
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

/* Test 4: Gang+worker partitioned (case 3) - nested gang/worker loops */
void test_gang_worker_partitioned() {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(G) num_workers(W) vector_length(1)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] = g * 100 + w * 10 + v;
                }
            }
        }
    }
}

/* Test 5: Vector partitioned (case 4) - explicit vector parallelism */
void test_vector_partitioned() {
    #pragma acc parallel copy(arr1d[0:N]) num_gangs(1) num_workers(1) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr1d[i] += i % 17;
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned() {
    #pragma acc parallel copy(arr2d[0:G][0:V]) num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang
        for (int g = 0; g < G; g++) {
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                arr2d[g][v] = (g << 16) | v;
            }
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned() {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int w = 0; w < W; w++) {
            #pragma acc loop vector
            for (int v = 0; v < V; v++) {
                arr2d[w][v] = (w << 8) + v;
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7) - gang, worker, and vector all active */
void test_fully_partitioned() {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang collapse(1)
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker collapse(1)
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector collapse(1)
                for (int v = 0; v < V; v++) {
                    arr3d[g][w][v] += g + w + v;
                }
            }
        }
    }
}

/* Test 9: Combined clauses to trigger various internal mappings */
void test_combined_clauses() {
    int temp[G][W][V];
    
    /* Different data clauses with shaping */
    #pragma acc data copy(temp[0:G][0:W][0:V])
    {
        #pragma acc parallel loop gang collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    temp[g][w][v] = 1;
                }
            }
        }
        
        #pragma acc parallel loop worker collapse(2)
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                temp[0][w][v] += 1;
            }
        }
        
        #pragma acc parallel loop vector
        for (int v = 0; v < V; v++) {
            temp[0][0][v] += 1;
        }
    }
    
    /* Verify computation */
    int check = 0;
    #pragma acc parallel copyin(temp[0:G][0:W][0:V]) copy(check)
    {
        #pragma acc loop gang reduction(+:check)
        for (int g = 0; g < G; g++) {
            #pragma acc loop worker reduction(+:check)
            for (int w = 0; w < W; w++) {
                #pragma acc loop vector reduction(+:check)
                for (int v = 0; v < V; v++) {
                    check += temp[g][w][v];
                }
            }
        }
    }
    
    global_sum += check;
}

int main() {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i % 256;
    }
    
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            arr2d[w][v] = w * 100 + v;
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
    printf("  Test 1 (gang redundant) complete\n");
    
    test_gang_partitioned();
    printf("  Test 2 (gang partitioned) complete\n");
    
    test_worker_partitioned();
    printf("  Test 3 (worker partitioned) complete\n");
    
    test_gang_worker_partitioned();
    printf("  Test 4 (gang+worker partitioned) complete\n");
    
    test_vector_partitioned();
    printf("  Test 5 (vector partitioned) complete\n");
    
    test_gang_vector_partitioned();
    printf("  Test 6 (gang+vector partitioned) complete\n");
    
    test_worker_vector_partitioned();
    printf("  Test 7 (worker+vector partitioned) complete\n");
    
    test_fully_partitioned();
    printf("  Test 8 (fully partitioned) complete\n");
    
    test_combined_clauses();
    printf("  Test 9 (combined clauses) complete\n");
    
    /* Compute final checksum */
    int final_check = global_sum;
    for (int i = 0; i < N; i++) {
        final_check += arr1d[i];
    }
    
    for (int w = 0; w < W; w++) {
        for (int v = 0; v < V; v++) {
            final_check += arr2d[w][v];
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
    printf("All tests completed. If compiled with offloading enabled,\n");
    printf("this should trigger all partition mapping cases (0-7).\n");
    
    return 0;
}
