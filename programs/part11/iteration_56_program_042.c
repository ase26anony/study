/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc
   Lines 335-343: case 0-7 return strings for different partition types
   Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
   Or for AMD: gcc -O2 -fopenacc -foffload=amdgcn-amdhsa -o test_partitions test_partitions.c
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
int array_3d[GANGS][WORKERS][VECTORS] = {0};

/* Test 1: Gang redundant (likely case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    int arr[N];
    
    /* Initialize array */
    for (int i = 0; i < N; i++) arr[i] = i;
    
    #pragma acc parallel copy(arr[0:N]) reduction(+:local_sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            local_sum += arr[i];
        }
    }
    
    global_sum += local_sum;
    printf("Gang redundant test completed, sum = %d\n", local_sum);
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    int arr[GANGS][WORKERS];
    
    /* Initialize 2D array */
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            arr[g][w] = g * 100 + w;
        }
    }
    
    #pragma acc parallel copy(arr[0:GANGS][0:WORKERS]) num_gangs(GANGS)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < WORKERS; w++) {
                arr[g][w] += 1;
            }
        }
    }
    
    /* Verify some values to prevent dead code elimination */
    int check = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            check += arr[g][w];
        }
    }
    global_sum += check;
    printf("Gang partitioned test completed, check = %d\n", check);
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    int arr[WORKERS][N/WORKERS];
    
    /* Initialize array */
    for (int w = 0; w < WORKERS; w++) {
        for (int i = 0; i < N/WORKERS; i++) {
            arr[w][i] = w * (N/WORKERS) + i;
        }
    }
    
    #pragma acc parallel copy(arr[0:WORKERS][0:N/WORKERS]) num_workers(WORKERS)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < WORKERS; w++) {
            #pragma acc loop vector
            for (int i = 0; i < N/WORKERS; i++) {
                arr[w][i] *= 2;
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
    printf("Worker partitioned test completed, check = %d\n", check);
}

/* Test 4: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    int arr[N];
    
    for (int i = 0; i < N; i++) arr[i] = i;
    
    #pragma acc parallel copy(arr[0:N]) vector_length(VECTORS)
    {
        #pragma acc loop vector independent
        for (int i = 0; i < N; i++) {
            arr[i] += 5;
        }
    }
    
    int check = 0;
    for (int i = 0; i < N; i++) check += arr[i];
    global_sum += check;
    printf("Vector partitioned test completed, check = %d\n", check);
}

/* Test 5: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    int arr[GANGS][WORKERS][10];
    
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int k = 0; k < 10; k++) {
                arr[g][w][k] = g * 1000 + w * 100 + k;
            }
        }
    }
    
    #pragma acc parallel copy(arr[0:GANGS][0:WORKERS][0:10]) \
        num_gangs(GANGS) num_workers(WORKERS)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector
                for (int k = 0; k < 10; k++) {
                    arr[g][w][k] -= 2;
                }
            }
        }
    }
    
    int check = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int k = 0; k < 10; k++) {
                check += arr[g][w][k];
            }
        }
    }
    global_sum += check;
    printf("Gang+worker partitioned test completed, check = %d\n", check);
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[GANGS][VECTORS * 16];
    
    for (int g = 0; g < GANGS; g++) {
        for (int v = 0; v < VECTORS * 16; v++) {
            arr[g][v] = g * 100 + v;
        }
    }
    
    #pragma acc parallel copy(arr[0:GANGS][0:VECTORS*16]) \
        num_gangs(GANGS) vector_length(VECTORS)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop vector independent
            for (int v = 0; v < VECTORS * 16; v++) {
                arr[g][v] = arr[g][v] * 3;
            }
        }
    }
    
    int check = 0;
    for (int g = 0; g < GANGS; g++) {
        for (int v = 0; v < VECTORS * 16; v++) {
            check += arr[g][v];
        }
    }
    global_sum += check;
    printf("Gang+vector partitioned test completed, check = %d\n", check);
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[WORKERS][VECTORS * 8];
    
    for (int w = 0; w < WORKERS; w++) {
        for (int v = 0; v < VECTORS * 8; v++) {
            arr[w][v] = w * 50 + v;
        }
    }
    
    #pragma acc parallel copy(arr[0:WORKERS][0:VECTORS*8]) \
        num_workers(WORKERS) vector_length(VECTORS)
    {
        #pragma acc loop worker independent
        for (int w = 0; w < WORKERS; w++) {
            #pragma acc loop vector independent
            for (int v = 0; v < VECTORS * 8; v++) {
                arr[w][v] += 10;
            }
        }
    }
    
    int check = 0;
    for (int w = 0; w < WORKERS; w++) {
        for (int v = 0; v < VECTORS * 8; v++) {
            check += arr[w][v];
        }
    }
    global_sum += check;
    printf("Worker+vector partitioned test completed, check = %d\n", check);
}

/* Test 8: Fully partitioned (case 7) - 3D array with all levels */
void test_fully_partitioned(void) {
    int arr[GANGS][WORKERS][VECTORS];
    
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                arr[g][w][v] = g * 10000 + w * 100 + v;
            }
        }
    }
    
    #pragma acc parallel copy(arr[0:GANGS][0:WORKERS][0:VECTORS]) \
        num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTORS)
    {
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < VECTORS; v++) {
                    arr[g][w][v] = arr[g][w][v] / 2;
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
    printf("Fully partitioned test completed, check = %d\n", check);
}

/* Test 9: Combined construct with collapse to trigger various partition mappings */
void test_combined_collapse(void) {
    int arr[GANGS][WORKERS][VECTORS];
    
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTORS; v++) {
                arr[g][w][v] = 1;
            }
        }
    }
    
    /* This collapsed loop may trigger different internal partition mappings */
    #pragma acc parallel copy(arr[0:GANGS][0:WORKERS][0:VECTORS]) \
        num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTORS)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < GANGS; g++) {
            for (int w = 0; w < WORKERS; w++) {
                for (int v = 0; v < VECTORS; v++) {
                    arr[g][w][v] *= 2;
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
    printf("Combined collapse test completed, check = %d\n", check);
}

int main(void) {
    printf("Starting OpenACC partition mapping tests...\n");
    
    /* Execute all test functions to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_vector_partitioned();
    test_gang_worker_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_combined_collapse();
    
    printf("\nAll tests completed. Global checksum = %d\n", global_sum);
    printf("If compiled with offloading, this should exercise the partition\n");
    printf("mapping function in omp-oacc-neuter-broadcast.cc lines 335-343.\n");
    
    return 0;
}
