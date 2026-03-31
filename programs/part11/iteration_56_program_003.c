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
    int sum = 0;
    #pragma acc parallel loop gang reduction(+:sum) copyin(arr1d[0:N]) copy(sum)
    for (int i = 0; i < N; i++) {
        sum += arr1d[i];
    }
    global_sum += sum;
}

/* Test 2: Gang partitioned (case 1) - gang distributed loop */
void test_gang_partitioned() {
    #pragma acc parallel loop gang copy(arr1d[0:N])
    for (int i = 0; i < N; i++) {
        arr1d[i] += 1;
    }
}

/* Test 3: Worker partitioned (case 2) - explicit worker loop */
void test_worker_partitioned() {
    #pragma acc parallel loop worker copy(arr2d[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] *= 2;
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3) - 2D distribution */
void test_gang_worker_partitioned() {
    #pragma acc parallel loop gang worker collapse(2) copy(arr3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
}

/* Test 5: Vector partitioned (case 4) - explicit vector loop */
void test_vector_partitioned() {
    #pragma acc parallel loop vector copy(arr1d[0:N])
    for (int i = 0; i < N; i++) {
        arr1d[i] = arr1d[i] * 3 + 1;
    }
}

/* Test 6: Gang+vector partitioned (case 5) - gang with vector lanes */
void test_gang_vector_partitioned() {
    #pragma acc parallel loop gang vector copy(arr1d[0:N])
    for (int i = 0; i < N; i++) {
        arr1d[i] = arr1d[i] << 1;
    }
}

/* Test 7: Worker+vector partitioned (case 6) - worker with vector lanes */
void test_worker_vector_partitioned() {
    #pragma acc parallel loop worker vector copy(arr2d[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = arr2d[i][j] | 0x01;
        }
    }
}

/* Test 8: Fully partitioned (case 7) - 3D collapsed loop */
void test_fully_partitioned() {
    #pragma acc parallel loop gang worker vector collapse(3) \
        copy(arr3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] += (i * j * k) % 7;
            }
        }
    }
}

/* Helper to initialize arrays */
void init_arrays() {
    for (int i = 0; i < N; i++) {
        arr1d[i] = i % 17;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = (i * V + j) % 23;
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = (i * W * V + j * V + k) % 31;
            }
        }
    }
}

/* Compute checksum to ensure all computations happened */
int compute_checksum() {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        sum = (sum + arr1d[i]) % 1000000007;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            sum = (sum + arr2d[i][j]) % 1000000007;
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                sum = (sum + arr3d[i][j][k]) % 1000000007;
            }
        }
    }
    
    return sum;
}

int main() {
    printf("Testing OpenACC partition mappings...\n");
    
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
    
    int checksum = compute_checksum();
    printf("Final checksum: %d\n", checksum);
    printf("Global sum (volatile): %d\n", (int)global_sum);
    
    /* Verify results are non-zero */
    if (checksum == 0 && global_sum == 0) {
        printf("WARNING: All results are zero - computations may have been optimized away\n");
        return 1;
    }
    
    printf("All partition tests completed.\n");
    return 0;
}
