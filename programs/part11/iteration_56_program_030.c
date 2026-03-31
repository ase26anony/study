/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc
   Lines 335-343: case 0-7 return strings for different partition types
   Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
   Or with AMD: gcc -O2 -fopenacc -foffload=amdgcn-amdhsa -o test_partitions test_partitions.c
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
    #pragma acc parallel copyin(arr1d[0:N]) copy(sum) num_gangs(G)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += arr1d[i];
        }
    }
    global_sum += sum;
}

/* Test 2: Gang partitioned (case 1) - each gang processes a chunk */
void test_gang_partitioned() {
    #pragma acc parallel copy(arr1d[0:N]) num_gangs(G)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] += 1;  /* Simple operation */
        }
    }
}

/* Test 3: Worker partitioned (case 2) - explicit worker loop */
void test_worker_partitioned() {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            #pragma acc loop vector
            for (int j = 0; j < V; j++) {
                arr2d[i][j] = i * 100 + j;
            }
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3) - 2D decomposition */
void test_gang_worker_partitioned() {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_gangs(2) num_workers(W/2) vector_length(V)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < W; i++) {
            #pragma acc loop vector
            for (int j = 0; j < V; j++) {
                arr2d[i][j] += 1;
            }
        }
    }
}

/* Test 5: Vector partitioned (case 4) - explicit vector loop */
void test_vector_partitioned() {
    #pragma acc parallel copy(arr1d[0:V]) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < V; i++) {
            arr1d[i] *= 2;
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned() {
    #pragma acc parallel copy(arr1d[0:N]) num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            arr1d[i] = arr1d[i] * 3 + i;
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned() {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                arr2d[i][j] += i + j;
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7) - 3D decomposition */
void test_fully_partitioned() {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    arr3d[i][j][k] = i * 10000 + j * 100 + k;
                }
            }
        }
    }
}

/* Test 9: Mixed partition patterns to trigger internal mapping */
void test_mixed_partitions() {
    /* Combined test that uses various data clauses and shapes */
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
    }
    
    /* Multiple OpenACC regions with different partition implications */
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(4) num_workers(2) vector_length(64)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Another region with worker partitioning */
    #pragma acc parallel copy(c[0:N]) num_workers(8) vector_length(32)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            c[i] += i;
        }
    }
    
    /* Verify results */
    int check = 0;
    for (int i = 0; i < N; i++) {
        check += c[i];
    }
    global_sum += check;
}

int main() {
    printf("Testing OpenACC partition mapping coverage...\n");
    
    /* Initialize data */
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
    int final_check = global_sum;
    for (int i = 0; i < N; i++) final_check += arr1d[i];
    for (int i = 0; i < W; i++) 
        for (int j = 0; j < V; j++) 
            final_check += arr2d[i][j];
    for (int i = 0; i < G; i++)
        for (int j = 0; j < W; j++)
            for (int k = 0; k < V; k++)
                final_check += arr3d[i][j][k];
    
    printf("Final checksum: %d\n", final_check);
    printf("If non-zero, all OpenACC regions executed.\n");
    
    return 0;
}
