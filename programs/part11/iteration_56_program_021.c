/* Test program to cover partition mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 * Or for AMD: gcc -O2 -fopenacc -foffload=amdgcn-amdhsa -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 2  /* vectors */
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int arr3d[G][W][V];
int arr2d[W][V];
int arr1d[N];

/* Function prototypes */
void test_gang_redundant(void);
void test_gang_partitioned(void);
void test_worker_partitioned(void);
void test_gang_worker_partitioned(void);
void test_vector_partitioned(void);
void test_gang_vector_partitioned(void);
void test_worker_vector_partitioned(void);
void test_fully_partitioned(void);

int main(void) {
    int i, j, k;
    
    /* Initialize arrays */
    for (i = 0; i < G; i++)
        for (j = 0; j < W; j++)
            for (k = 0; k < V; k++)
                arr3d[i][j][k] = i * 100 + j * 10 + k;
    
    for (j = 0; j < W; j++)
        for (k = 0; k < V; k++)
            arr2d[j][k] = j * 10 + k;
    
    for (i = 0; i < N; i++)
        arr1d[i] = i;
    
    printf("Testing various OpenACC partition patterns...\n");
    
    /* Test each partition type to trigger different internal codes */
    test_gang_redundant();           /* Should trigger case 0 */
    test_gang_partitioned();         /* Should trigger case 1 */
    test_worker_partitioned();       /* Should trigger case 2 */
    test_gang_worker_partitioned();  /* Should trigger case 3 */
    test_vector_partitioned();       /* Should trigger case 4 */
    test_gang_vector_partitioned();  /* Should trigger case 5 */
    test_worker_vector_partitioned();/* Should trigger case 6 */
    test_fully_partitioned();        /* Should trigger case 7 */
    
    /* Verify results by computing checksum */
    int checksum = 0;
    #pragma acc parallel loop reduction(+:checksum) copyin(arr1d[0:N])
    for (i = 0; i < N; i++) {
        checksum += arr1d[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global sum (volatile): %d\n", (int)global_sum);
    
    return 0;
}

/* Case 0: Gang redundant - scalar reduction across gangs */
void test_gang_redundant(void) {
    int sum = 0;
    
    #pragma acc parallel copyin(arr1d[0:N]) copy(sum) \
        num_gangs(G) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += arr1d[i];
        }
    }
    
    global_sum += sum;
}

/* Case 1: Gang partitioned - each gang works on separate chunk */
void test_gang_partitioned(void) {
    int partial_sums[G] = {0};
    
    #pragma acc parallel copyin(arr1d[0:N]) copyout(partial_sums[0:G]) \
        num_gangs(G) num_workers(1) vector_length(1)
    {
        int gang_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        #endif
        
        #pragma acc loop gang
        for (int i = gang_id * (N/G); i < (gang_id + 1) * (N/G); i++) {
            if (i < N) partial_sums[gang_id] += arr1d[i];
        }
    }
    
    for (int i = 0; i < G; i++) {
        global_sum += partial_sums[i];
    }
}

/* Case 2: Worker partitioned - workers within gangs process data */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) \
        num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr2d[j][k] += 1;  /* Each worker updates its own row */
            }
        }
    }
    
    /* Update global sum from modified array */
    #pragma acc parallel loop collapse(2) reduction(+:global_sum) \
        copyin(arr2d[0:W][0:V])
    for (int j = 0; j < W; j++) {
        for (int k = 0; k < V; k++) {
            global_sum += arr2d[j][k];
        }
    }
}

/* Case 3: Gang+worker partitioned - 2D decomposition */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(1)
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    arr3d[i][j][k] *= 2;
                }
            }
        }
    }
}

/* Case 4: Vector partitioned - vector operations */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N]) \
        num_gangs(1) num_workers(1) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr1d[i] += i % 7;  /* Vector operation */
        }
    }
}

/* Case 5: Gang+vector partitioned */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:1][0:V]) \
        num_gangs(G) num_workers(1) vector_length(V)
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < G; i++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][0][k] = i * V + k;
            }
        }
    }
}

/* Case 6: Worker+vector partitioned */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) \
        num_gangs(1) num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector collapse(2)
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr2d[j][k] = j * V + k;
            }
        }
    }
}

/* Case 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    arr3d[i][j][k] = 1;
                }
            }
        }
    }
    
    /* Verify the update */
    int verify_sum = 0;
    #pragma acc parallel loop collapse(3) reduction(+:verify_sum) \
        copyin(arr3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                verify_sum += arr3d[i][j][k];
            }
        }
    }
    
    global_sum += verify_sum;
}
