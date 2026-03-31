/* test_oacc_partition.c
 * 
 * This program exercises GCC's OpenACC partitioning logic to trigger
 * the switch cases in omp-oacc-neuter-broadcast.cc lines 335-343.
 * Each test function targets a specific partitioning pattern.
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define DIM 32

/* Test 1: Gang redundant (case 0)
 * A parallel region without an associated loop, or with gang(1) */
static void test_gang_redundant(int *flag) {
    *flag = 0;
    #pragma acc parallel copy(flag)
    {
        #pragma acc loop gang(static:1)
        for (int i = 0; i < 1; ++i) {
            *flag = 1;
        }
    }
}

/* Test 2: Gang partitioned (case 1)
 * Single loop with explicit gang partitioning */
static void test_gang_partitioned(float *arr, int n) {
    float sum = 0.0f;
    #pragma acc parallel loop gang copy(arr[0:n]) reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        arr[i] = (float)i * 2.0f;
        sum += arr[i];
    }
    /* Use sum to prevent elimination */
    arr[0] += sum * 0.001f;
}

/* Test 3: Worker partitioned (case 2)
 * Nested loops with worker partitioning on inner loop */
static void test_worker_partitioned(float *arr, int n) {
    int workers = 4;
    #pragma acc parallel copy(arr[0:n]) num_workers(workers)
    {
        #pragma acc loop gang
        for (int g = 0; g < workers; ++g) {
            #pragma acc loop worker
            for (int w = 0; w < n/workers; ++w) {
                int idx = g * (n/workers) + w;
                if (idx < n) {
                    arr[idx] = arr[idx] * 1.5f + (float)w;
                }
            }
        }
    }
}

/* Test 4: Vector partitioned (case 4)
 * Loop with explicit vector partitioning */
static void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n])
    for (int i = 0; i < n; ++i) {
        arr[i] = arr[i] / 3.14159f;
    }
}

/* Test 5: Gang+worker partitioned (case 3)
 * Nested loops with gang and worker partitioning */
static void test_gang_worker_partitioned(float arr[DIM][DIM]) {
    #pragma acc parallel copy(arr[0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (int i = 0; i < DIM; ++i) {
            #pragma acc loop worker
            for (int j = 0; j < DIM; ++j) {
                arr[i][j] = (float)(i + j) * 0.5f;
            }
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5)
 * Nested loops with gang and vector partitioning */
static void test_gang_vector_partitioned(float arr[DIM][DIM]) {
    #pragma acc parallel copy(arr[0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (int i = 0; i < DIM; ++i) {
            #pragma acc loop vector
            for (int j = 0; j < DIM; ++j) {
                arr[i][j] = arr[i][j] * 2.0f - (float)j;
            }
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6)
 * Nested loops with worker and vector partitioning */
static void test_worker_vector_partitioned(float arr[DIM][DIM]) {
    #pragma acc parallel copy(arr[0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (int g = 0; g < 2; ++g) {
            #pragma acc loop worker vector
            for (int i = 0; i < DIM*DIM/2; ++i) {
                int idx = g * (DIM*DIM/2) + i;
                if (idx < DIM*DIM) {
                    int x = idx / DIM;
                    int y = idx % DIM;
                    arr[x][y] = arr[x][y] + (float)(x * y);
                }
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7)
 * Triple-nested loops with gang, worker, and vector partitioning */
static void test_fully_partitioned(float arr[DIM][DIM]) {
    float temp[DIM][DIM];
    
    #pragma acc parallel copy(arr[0:DIM][0:DIM]) create(temp[0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (int i = 0; i < DIM; ++i) {
            #pragma acc loop worker
            for (int j = 0; j < DIM; ++j) {
                float sum = 0.0f;
                #pragma acc loop vector reduction(+:sum)
                for (int k = 0; k < DIM; ++k) {
                    sum += arr[i][k] * arr[k][j];
                }
                temp[i][j] = sum;
            }
        }
        
        #pragma acc loop gang
        for (int i = 1; i < DIM-1; ++i) {
            #pragma acc loop worker
            for (int j = 1; j < DIM-1; ++j) {
                #pragma acc loop vector
                for (int k = 0; k < 3; ++k) {
                    /* Stencil-like computation */
                    arr[i][j] = (temp[i-1][j] + temp[i][j-1] + 
                                temp[i+1][j] + temp[i][j+1]) * 0.25f;
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Initialize test data */
    float arr1[SIZE];
    float arr2[DIM][DIM];
    int flag = 0;
    
    for (int i = 0; i < SIZE; ++i) {
        arr1[i] = (float)i;
    }
    
    for (int i = 0; i < DIM; ++i) {
        for (int j = 0; j < DIM; ++j) {
            arr2[i][j] = (float)(i * DIM + j);
        }
    }
    
    /* Use argc to control which tests run, ensuring all code paths are compiled */
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(&flag);
            printf("Gang redundant test: flag = %d\n", flag);
            break;
        case 1:
            test_gang_partitioned(arr1, SIZE);
            printf("Gang partitioned: arr1[0]=%.2f, arr1[%d]=%.2f\n", 
                   arr1[0], SIZE-1, arr1[SIZE-1]);
            break;
        case 2:
            test_worker_partitioned(arr1, SIZE);
            printf("Worker partitioned: arr1[0]=%.2f\n", arr1[0]);
            break;
        case 3:
            test_gang_worker_partitioned(arr2);
            printf("Gang+worker partitioned: arr2[0][0]=%.2f\n", arr2[0][0]);
            break;
        case 4:
            test_vector_partitioned(arr1, SIZE);
            printf("Vector partitioned: arr1[0]=%.2f\n", arr1[0]);
            break;
        case 5:
            test_gang_vector_partitioned(arr2);
            printf("Gang+vector partitioned: arr2[0][0]=%.2f\n", arr2[0][0]);
            break;
        case 6:
            test_worker_vector_partitioned(arr2);
            printf("Worker+vector partitioned: arr2[0][0]=%.2f\n", arr2[0][0]);
            break;
        case 7:
            test_fully_partitioned(arr2);
            printf("Fully partitioned: arr2[1][1]=%.2f, arr2[%d][%d]=%.2f\n",
                   arr2[1][1], DIM-2, DIM-2, arr2[DIM-2][DIM-2]);
            break;
        default:
            /* Run all tests when no specific case is selected */
            test_gang_redundant(&flag);
            test_gang_partitioned(arr1, SIZE);
            test_worker_partitioned(arr1, SIZE/2);
            test_vector_partitioned(arr1, SIZE);
            test_gang_worker_partitioned(arr2);
            test_gang_vector_partitioned(arr2);
            test_worker_vector_partitioned(arr2);
            test_fully_partitioned(arr2);
            
            printf("All tests executed:\n");
            printf("  flag=%d, arr1[0]=%.2f, arr2[0][0]=%.2f\n",
                   flag, arr1[0], arr2[0][0]);
            break;
    }
    
    return 0;
}
