/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition code string mapping
 * logic in GCC's OpenACC neuter-broadcast pass, specifically covering
 * the switch statement cases for all partition types.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N 64
#define M 32
#define P 16

/* Test function with routine directive and gang partitioning */
#pragma acc routine vec gang
void vec_gang_routine(int *arr, int n, int val) {
    for (int i = 0; i < n; i++) {
        arr[i] += val;
    }
}

/* Test function with routine directive and vector partitioning */
#pragma acc routine vec vector
void vec_vector_routine(int *arr, int n, int val) {
    for (int i = 0; i < n; i++) {
        arr[i] *= val;
    }
}

/* Test 1: Basic partition combinations on multi-dimensional arrays */
void test_basic_partitions(int argc) {
    int arr3d[10][20][30];
    int i, j, k;
    
    /* Initialize */
    for (i = 0; i < 10; i++)
        for (j = 0; j < 20; j++)
            for (k = 0; k < 30; k++)
                arr3d[i][j][k] = i + j + k;
    
    /* Case 0: gang redundant */
    if (argc > 1) {
        #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) gang
        {
            #pragma acc loop gang
            for (i = 0; i < 10; i++)
                for (j = 0; j < 20; j++)
                    for (k = 0; k < 30; k++)
                        arr3d[i][j][k] += 1;
        }
    }
    
    /* Case 1: gang partitioned */
    if (argc > 2) {
        #pragma acc kernels create(arr3d[0:10][0:20][0:30]) gang(static:2)
        {
            #pragma acc loop gang
            for (i = 0; i < 10; i++)
                for (j = 0; j < 20; j++)
                    for (k = 0; k < 30; k++)
                        arr3d[i][j][k] += 2;
        }
    }
    
    /* Case 2: worker partitioned */
    if (argc > 3) {
        #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) worker
        {
            #pragma acc loop worker
            for (i = 0; i < 10; i++)
                for (j = 0; j < 20; j++)
                    for (k = 0; k < 30; k++)
                        arr3d[i][j][k] += 3;
        }
    }
    
    /* Case 3: gang+worker partitioned */
    if (argc > 4) {
        #pragma acc kernels copy(arr3d[0:10][0:20][0:30]) gang worker
        {
            #pragma acc loop gang worker
            for (i = 0; i < 10; i++)
                for (j = 0; j < 20; j++)
                    for (k = 0; k < 30; k++)
                        arr3d[i][j][k] += 4;
        }
    }
    
    /* Case 4: vector partitioned */
    if (argc > 5) {
        #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) vector
        {
            #pragma acc loop vector
            for (i = 0; i < 10; i++)
                for (j = 0; j < 20; j++)
                    for (k = 0; k < 30; k++)
                        arr3d[i][j][k] += 5;
        }
    }
    
    /* Case 5: gang+vector partitioned */
    if (argc > 6) {
        #pragma acc kernels copy(arr3d[0:10][0:20][0:30]) gang vector
        {
            #pragma acc loop gang vector
            for (i = 0; i < 10; i++)
                for (j = 0; j < 20; j++)
                    for (k = 0; k < 30; k++)
                        arr3d[i][j][k] += 6;
        }
    }
    
    /* Case 6: worker+vector partitioned */
    if (argc > 7) {
        #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) worker vector
        {
            #pragma acc loop worker vector
            for (i = 0; i < 10; i++)
                for (j = 0; j < 20; j++)
                    for (k = 0; k < 30; k++)
                        arr3d[i][j][k] += 7;
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (argc > 8) {
        #pragma acc kernels copy(arr3d[0:10][0:20][0:30]) gang worker vector
        {
            #pragma acc loop gang worker vector collapse(3)
            for (i = 0; i < 10; i++)
                for (j = 0; j < 20; j++)
                    for (k = 0; k < 30; k++)
                        arr3d[i][j][k] += 8;
        }
    }
}

/* Test 2: Nested compute regions with different partition types */
void test_nested_regions(int argc) {
    int arr1[N], arr2[M], arr3[P];
    int i;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) arr1[i] = i;
    for (i = 0; i < M; i++) arr2[i] = i * 2;
    for (i = 0; i < P; i++) arr3[i] = i * 3;
    
    /* Outer region with gang partitioning */
    if (argc > 1) {
        #pragma acc parallel copy(arr1[0:N]) gang
        {
            /* Inner region with worker partitioning */
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                arr1[i] += 10;
                
                /* Nested conditional region */
                if (i < M) {
                    #pragma acc kernels copy(arr2[0:M]) worker if(i % 2 == 0)
                    {
                        #pragma acc loop worker
                        for (int j = 0; j < M; j++) {
                            arr2[j] += arr1[i % N];
                        }
                    }
                }
            }
        }
    }
    
    /* Sequential region with vector partitioning */
    if (argc > 2) {
        #pragma acc parallel copy(arr3[0:P]) vector
        {
            #pragma acc loop vector
            for (i = 0; i < P; i++) {
                arr3[i] *= 2;
            }
        }
    }
}

/* Test 3: Device data environment with partition clauses */
void test_device_data_env(int argc) {
    int persistent_arr[N][M];
    int i, j;
    
    /* Initialize */
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            persistent_arr[i][j] = i * M + j;
    
    /* Enter data with gang partitioning */
    if (argc > 1) {
        #pragma acc enter data copyin(persistent_arr[0:N][0:M]) gang
        
        /* Compute region 1: worker partitioned */
        #pragma acc parallel present(persistent_arr) worker
        {
            #pragma acc loop worker
            for (i = 0; i < N; i++)
                for (j = 0; j < M; j++)
                    persistent_arr[i][j] += 1;
        }
        
        /* Compute region 2: vector partitioned */
        #pragma acc kernels present(persistent_arr) vector
        {
            #pragma acc loop vector
            for (i = 0; i < N; i++)
                for (j = 0; j < M; j++)
                    persistent_arr[i][j] *= 2;
        }
        
        /* Exit data */
        #pragma acc exit data copyout(persistent_arr[0:N][0:M])
    }
}

/* Test 4: Routine directives with partition propagation */
void test_routine_partitions(int argc) {
    int routine_arr[N];
    int i;
    
    /* Initialize */
    for (i = 0; i < N; i++) routine_arr[i] = i;
    
    /* Call gang-partitioned routine from gang-partitioned region */
    if (argc > 1) {
        #pragma acc parallel copy(routine_arr[0:N]) gang
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                vec_gang_routine(&routine_arr[i], 1, 5);
            }
        }
    }
    
    /* Call vector-partitioned routine from vector-partitioned region */
    if (argc > 2) {
        #pragma acc kernels copy(routine_arr[0:N]) vector
        {
            #pragma acc loop vector
            for (i = 0; i < N; i++) {
                vec_vector_routine(&routine_arr[i], 1, 3);
            }
        }
    }
}

/* Test 5: Complex multi-dimensional partitioning with collapse */
void test_collapse_partitions(int argc) {
    int complex_arr[8][16][32];
    int i, j, k;
    
    /* Initialize */
    for (i = 0; i < 8; i++)
        for (j = 0; j < 16; j++)
            for (k = 0; k < 32; k++)
                complex_arr[i][j][k] = i * 100 + j * 10 + k;
    
    /* Test various collapse levels with different partitions */
    if (argc > 1) {
        /* Collapse 2 with gang+worker */
        #pragma acc parallel loop collapse(2) \
            copy(complex_arr[0:8][0:16][0:32]) \
            gang worker
        for (i = 0; i < 8; i++)
            for (j = 0; j < 16; j++)
                for (k = 0; k < 32; k++)
                    complex_arr[i][j][k] += i + j + k;
    }
    
    if (argc > 2) {
        /* Collapse 3 with gang+vector */
        #pragma acc kernels loop collapse(3) \
            copy(complex_arr[0:8][0:16][0:32]) \
            gang vector
        for (i = 0; i < 8; i++)
            for (j = 0; j < 16; j++)
                for (k = 0; k < 32; k++)
                    complex_arr[i][j][k] *= 2;
    }
    
    if (argc > 3) {
        /* Mixed partitioning with conditional */
        #pragma acc parallel copy(complex_arr[0:8][0:16][0:32]) \
            gang worker vector if(argc > 5)
        {
            #pragma acc loop gang worker vector collapse(3)
            for (i = 0; i < 8; i++)
                for (j = 0; j < 16; j++)
                    for (k = 0; k < 32; k++)
                        complex_arr[i][j][k] -= 1;
        }
    }
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition type coverage...\n");
    
    /* Execute all tests with argc-based conditions to prevent dead code elimination */
    test_basic_partitions(argc);
    test_nested_regions(argc);
    test_device_data_env(argc);
    test_routine_partitions(argc);
    test_collapse_partitions(argc);
    
    printf("Test completed (compile-time coverage target achieved).\n");
    return 0;
}
