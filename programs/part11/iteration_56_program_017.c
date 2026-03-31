/* test_openacc_partitions.c
 * Designed to exercise GCC's internal partition mapping function
 * for OpenACC data partitioning across gang, worker, and vector levels.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GANGS 8
#define WORKERS 4
#define VECTORS 2
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int global_array[N] = {0};

/* Test 1: Gang redundant partitioning (likely case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    int arr[N];
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i % 100;
    }
    
    /* OpenACC parallel region - likely gang redundant */
    #pragma acc parallel copy(arr[0:N]) copyout(local_sum)
    {
        int sum = 0;
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
        local_sum = sum;
    }
    
    global_sum += local_sum;
    printf("Gang redundant test completed, sum: %d\n", local_sum);
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int arr[GANGS][WORKERS];
    
    /* Initialize 2D array */
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            arr[g][w] = g * 10 + w;
        }
    }
    
    /* OpenACC with explicit gang partitioning */
    #pragma acc parallel copy(arr[0:GANGS][0:WORKERS]) num_gangs(GANGS)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < WORKERS; w++) {
                arr[g][w] += 1;  /* Simple computation */
            }
        }
    }
    
    /* Verify some results */
    int check = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            check += arr[g][w];
        }
    }
    global_sum += check;
    printf("Gang partitioned test completed, check: %d\n", check);
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[WORKERS][N/WORKERS];
    
    /* Initialize array */
    for (int w = 0; w < WORKERS; w++) {
        for (int i = 0; i < N/WORKERS; i++) {
            arr[w][i] = w * 100 + i;
        }
    }
    
    /* Explicit worker partitioning */
    #pragma acc parallel copy(arr[0:WORKERS][0:N/WORKERS]) num_workers(WORKERS)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < WORKERS; w++) {
            #pragma acc loop vector independent
            for (int i = 0; i < N/WORKERS; i++) {
                arr[w][i] *= 2;  /* Worker-specific computation */
            }
        }
    }
    
    int check = 0;
    for (int w = 0; w < WORKERS; w++) {
        for (int i = 0; i < N/WORKERS; i++) {
            check += arr[w][i];
        }
    }
    global_sum += check;
    printf("Worker partitioned test completed, check: %d\n", check);
}

/* Test 4: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[N];
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    /* Explicit vector partitioning */
    #pragma acc parallel copy(arr[0:N]) vector_length(VECTORS)
    {
        #pragma acc loop vector independent
        for (int i = 0; i < N; i++) {
            arr[i] = arr[i] * 3 + 1;  /* Vector operation */
        }
    }
    
    int check = 0;
    for (int i = 0; i < N; i++) {
        check += arr[i];
    }
    global_sum += check;
    printf("Vector partitioned test completed, check: %d\n", check);
}

/* Test 5: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[GANGS][WORKERS][VECTORS];
    
    /* Initialize 3D array */
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                arr[g][w][v] = g * 100 + w * 10 + v;
            }
        }
    }
    
    /* Combined gang and worker partitioning */
    #pragma acc parallel copy(arr[0:GANGS][0:WORKERS][0:VECTORS]) \
        num_gangs(GANGS) num_workers(WORKERS)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < VECTORS; v++) {
                    arr[g][w][v] += g + w + v;
                }
            }
        }
    }
    
    int check = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                check += arr[g][w][v];
            }
        }
    }
    global_sum += check;
    printf("Gang+worker partitioned test completed, check: %d\n", check);
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[GANGS][VECTORS][N/(GANGS*VECTORS)];
    
    /* Initialize array */
    for (int g = 0; g < GANGS; g++) {
        for (int v = 0; v < VECTORS; v++) {
            for (int i = 0; i < N/(GANGS*VECTORS); i++) {
                arr[g][v][i] = g * VECTORS + v + i;
            }
        }
    }
    
    /* Gang and vector partitioning */
    #pragma acc parallel copy(arr[0:GANGS][0:VECTORS][0:N/(GANGS*VECTORS)]) \
        num_gangs(GANGS) vector_length(VECTORS)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop vector independent
            for (int v = 0; v < VECTORS; v++) {
                for (int i = 0; i < N/(GANGS*VECTORS); i++) {
                    arr[g][v][i] = arr[g][v][i] * 2 - 1;
                }
            }
        }
    }
    
    int check = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int v = 0; v < VECTORS; v++) {
            for (int i = 0; i < N/(GANGS*VECTORS); i++) {
                check += arr[g][v][i];
            }
        }
    }
    global_sum += check;
    printf("Gang+vector partitioned test completed, check: %d\n", check);
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[WORKERS][VECTORS][N/(WORKERS*VECTORS)];
    
    /* Initialize array */
    for (int w = 0; w < WORKERS; w++) {
        for (int v = 0; v < VECTORS; v++) {
            for (int i = 0; i < N/(WORKERS*VECTORS); i++) {
                arr[w][v][i] = w * VECTORS + v + i * 2;
            }
        }
    }
    
    /* Worker and vector partitioning */
    #pragma acc parallel copy(arr[0:WORKERS][0:VECTORS][0:N/(WORKERS*VECTORS)]) \
        num_workers(WORKERS) vector_length(VECTORS)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < WORKERS; w++) {
            #pragma acc loop vector independent
            for (int v = 0; v < VECTORS; v++) {
                for (int i = 0; i < N/(WORKERS*VECTORS); i++) {
                    arr[w][v][i] += (w * v + i) % 7;
                }
            }
        }
    }
    
    int check = 0;
    for (int w = 0; w < WORKERS; w++) {
        for (int v = 0; v < VECTORS; v++) {
            for (int i = 0; i < N/(WORKERS*VECTORS); i++) {
                check += arr[w][v][i];
            }
        }
    }
    global_sum += check;
    printf("Worker+vector partitioned test completed, check: %d\n", check);
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    int arr[GANGS][WORKERS][VECTORS];
    
    /* Initialize 3D array */
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                arr[g][w][v] = 1;
            }
        }
    }
    
    /* Fully partitioned across all levels */
    #pragma acc parallel copy(arr[0:GANGS][0:WORKERS][0:VECTORS]) \
        num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTORS)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < VECTORS; v++) {
                    /* Each element processed independently */
                    arr[g][w][v] = g * WORKERS * VECTORS + w * VECTORS + v + 100;
                }
            }
        }
    }
    
    int check = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                check += arr[g][w][v];
            }
        }
    }
    global_sum += check;
    printf("Fully partitioned test completed, check: %d\n", check);
}

/* Test 9: Complex collapse with all partition types */
void test_collapsed_partitions(void) {
    int arr[GANGS][WORKERS][VECTORS];
    
    /* Initialize */
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                arr[g][w][v] = 0;
            }
        }
    }
    
    /* Collapsed loop with multiple partition levels */
    #pragma acc parallel copy(arr[0:GANGS][0:WORKERS][0:VECTORS]) \
        num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTORS)
    {
        #pragma acc loop gang worker vector collapse(3) independent
        for (int g = 0; g < GANGS; g++) {
            for (int w = 0; w < WORKERS; w++) {
                for (int v = 0; v < VECTORS; v++) {
                    arr[g][w][v] = (g << 16) | (w << 8) | v;
                }
            }
        }
    }
    
    int check = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                check += arr[g][w][v];
            }
        }
    }
    global_sum += check;
    printf("Collapsed partitions test completed, check: %d\n", check);
}

int main(void) {
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_vector_partitioned();
    test_gang_worker_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_collapsed_partitions();
    
    /* Final verification */
    printf("\nAll tests completed. Global checksum: %d\n", global_sum);
    printf("Expected non-zero checksum if all tests executed.\n");
    
    /* Additional simple test to ensure code generation */
    #pragma acc parallel loop copy(global_array[0:N])
    for (int i = 0; i < N; i++) {
        global_array[i] = i * 2;
    }
    
    int final_check = 0;
    for (int i = 0; i < N; i++) {
        final_check += global_array[i];
    }
    printf("Final array checksum: %d\n", final_check);
    
    return 0;
}
