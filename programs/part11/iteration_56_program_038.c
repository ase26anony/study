/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 * Or for AMD: gcc -O2 -fopenacc -foffload=amdgcn-amdhsa -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 32 /* vector length */
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int output_array[N] = {0};

/* Test 1: Gang redundant (case 0) - scalar reduction */
void test_gang_redundant(void) {
    int arr[N];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i % 100;
    }
    
    #pragma acc parallel copyin(arr[0:N]) copy(sum) num_gangs(G)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
    }
    
    global_sum += sum;
    printf("Gang redundant test: sum = %d\n", sum);
}

/* Test 2: Gang partitioned (case 1) - 1D array partitioned across gangs */
void test_gang_partitioned(void) {
    int arr[N];
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    #pragma acc parallel copy(arr[0:N]) num_gangs(G)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr[i] *= 2;
        }
    }
    
    /* Verify some values to prevent dead code elimination */
    int check = 0;
    for (int i = 0; i < 10; i++) {
        check += arr[i];
    }
    global_sum += check;
    printf("Gang partitioned test: check = %d\n", check);
}

/* Test 3: Worker partitioned (case 2) - 2D array with worker parallelism */
void test_worker_partitioned(void) {
    int arr[W][V];
    
    /* Initialize 2D array */
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr[i][j] = i * V + j;
        }
    }
    
    #pragma acc parallel copy(arr[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            #pragma acc loop vector
            for (int j = 0; j < V; j++) {
                arr[i][j] += 1;
            }
        }
    }
    
    int check = 0;
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            check += arr[i][j];
        }
    }
    global_sum += check;
    printf("Worker partitioned test: check = %d\n", check);
}

/* Test 4: Gang+worker partitioned (case 3) - 3D array with gang and worker parallelism */
void test_gang_worker_partitioned(void) {
    int arr[G][W][V/8];  /* Smaller 3rd dimension */
    
    /* Initialize 3D array */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V/8; k++) {
                arr[i][j][k] = i * W * (V/8) + j * (V/8) + k;
            }
        }
    }
    
    #pragma acc parallel copy(arr[0:G][0:W][0:V/8]) \
        num_gangs(G) num_workers(W) vector_length(V/8)
    {
        #pragma acc loop gang
        for (int i = 0; i < G; i++) {
            #pragma acc loop worker
            for (int j = 0; j < W; j++) {
                #pragma acc loop vector
                for (int k = 0; k < V/8; k++) {
                    arr[i][j][k] *= 3;
                }
            }
        }
    }
    
    int check = 0;
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V/8; k++) {
                check += arr[i][j][k];
            }
        }
    }
    global_sum += check;
    printf("Gang+worker partitioned test: check = %d\n", check);
}

/* Test 5: Vector partitioned (case 4) - vector operations */
void test_vector_partitioned(void) {
    int arr[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    #pragma acc parallel copy(arr[0:N]) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr[i] = arr[i] + arr[i] * 2;
        }
    }
    
    int check = 0;
    for (int i = 0; i < N; i += 16) {
        check += arr[i];
    }
    global_sum += check;
    printf("Vector partitioned test: check = %d\n", check);
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    int arr[G][V];
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < V; j++) {
            arr[i][j] = i * V + j;
        }
    }
    
    #pragma acc parallel copy(arr[0:G][0:V]) num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang
        for (int i = 0; i < G; i++) {
            #pragma acc loop vector
            for (int j = 0; j < V; j++) {
                arr[i][j] += 5;
            }
        }
    }
    
    int check = 0;
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < V; j++) {
            check += arr[i][j];
        }
    }
    global_sum += check;
    printf("Gang+vector partitioned test: check = %d\n", check);
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    int arr[W][V];
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr[i][j] = i * V + j;
        }
    }
    
    #pragma acc parallel copy(arr[0:W][0:V]) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            #pragma acc loop vector
            for (int j = 0; j < V; j++) {
                arr[i][j] -= 2;
            }
        }
    }
    
    int check = 0;
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            check += arr[i][j];
        }
    }
    global_sum += check;
    printf("Worker+vector partitioned test: check = %d\n", check);
}

/* Test 8: Fully partitioned (case 7) - 3D array with all levels of parallelism */
void test_fully_partitioned(void) {
    int arr[G][W][V/16];
    
    /* Initialize */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V/16; k++) {
                arr[i][j][k] = i * W * (V/16) + j * (V/16) + k;
            }
        }
    }
    
    /* Complex data clause with shaping to encourage partition analysis */
    #pragma acc parallel copy(arr[0:G][0:W][0:V/16]) \
        num_gangs(G) num_workers(W) vector_length(V/16)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V/16; k++) {
                    arr[i][j][k] = (arr[i][j][k] * 7) / 3;
                }
            }
        }
    }
    
    int check = 0;
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V/16; k++) {
                check += arr[i][j][k];
            }
        }
    }
    global_sum += check;
    printf("Fully partitioned test: check = %d\n", check);
}

/* Test 9: Mixed constructs to trigger various internal mappings */
void test_mixed_constructs(void) {
    /* Use kernels directive with different data mappings */
    int arr1[N], arr2[N/2], arr3[N/4];
    
    for (int i = 0; i < N; i++) arr1[i] = i;
    for (int i = 0; i < N/2; i++) arr2[i] = i * 2;
    for (int i = 0; i < N/4; i++) arr3[i] = i * 3;
    
    #pragma acc kernels copy(arr1[0:N], arr2[0:N/2], arr3[0:N/4]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1[i] += 1;
        }
        
        #pragma acc loop worker
        for (int i = 0; i < N/2; i++) {
            arr2[i] *= 2;
        }
        
        #pragma acc loop vector
        for (int i = 0; i < N/4; i++) {
            arr3[i] -= 3;
        }
    }
    
    int check = arr1[10] + arr2[5] + arr3[2];
    global_sum += check;
    printf("Mixed constructs test: check = %d\n", check);
}

int main(void) {
    printf("Starting OpenACC partition mapping tests...\n");
    
    /* Execute all test functions to trigger different partition mappings */
    test_gang_redundant();           /* Should trigger case 0 */
    test_gang_partitioned();         /* Should trigger case 1 */
    test_worker_partitioned();       /* Should trigger case 2 */
    test_gang_worker_partitioned();  /* Should trigger case 3 */
    test_vector_partitioned();       /* Should trigger case 4 */
    test_gang_vector_partitioned();  /* Should trigger case 5 */
    test_worker_vector_partitioned();/* Should trigger case 6 */
    test_fully_partitioned();        /* Should trigger case 7 */
    test_mixed_constructs();         /* May trigger multiple cases */
    
    printf("\nAll tests completed. Global checksum: %d\n", global_sum);
    printf("(Non-zero checksum indicates computations were performed)\n");
    
    return 0;
}
