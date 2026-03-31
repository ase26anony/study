/* test_omp_acc_partitions.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 50
#define P 25

/* Routine with explicit partition specification */
#pragma acc routine vec gang
void increment_element(int *arr, int idx, int val) {
    arr[idx] += val;
}

/* Function with nested compute regions */
void test_nested_partitions(int *data, int size) {
    int i;
    
    /* Outer region with gang partitioning */
    #pragma acc parallel copy(data[0:size]) gang
    {
        /* Inner region with worker partitioning */
        #pragma acc loop worker
        for (i = 0; i < size; i++) {
            data[i] = i;
        }
    }
}

/* Function with multi-dimensional array partitioning */
void test_multi_dim_partitions(int arr3d[10][20][30]) {
    int i, j, k;
    
    /* Fully partitioned (gang+worker+vector) */
    #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) gang worker vector
    {
        #pragma acc loop collapse(3) gang
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 20; j++) {
                for (k = 0; k < 30; k++) {
                    arr3d[i][j][k] = i * j * k;
                }
            }
        }
    }
    
    /* Worker+vector partitioned */
    #pragma acc kernels copy(arr3d[0:10][0:20][0:30]) worker vector
    {
        #pragma acc loop collapse(2) worker
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 20; j++) {
                #pragma acc loop vector
                for (k = 0; k < 30; k++) {
                    arr3d[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test function with various partition combinations */
void test_partition_combinations(int *arr1, int *arr2, int size, int use_alt) {
    int i;
    
    if (use_alt) {
        /* Gang partitioned */
        #pragma acc parallel copy(arr1[0:size]) gang(static:2)
        {
            #pragma acc loop gang
            for (i = 0; i < size; i++) {
                arr1[i] *= 2;
            }
        }
        
        /* Vector partitioned */
        #pragma acc kernels copy(arr2[0:size]) vector
        {
            #pragma acc loop vector
            for (i = 0; i < size; i++) {
                arr2[i] += arr1[i % size];
            }
        }
    } else {
        /* Gang+vector partitioned */
        #pragma acc parallel copy(arr1[0:size]) gang vector
        {
            #pragma acc loop gang vector
            for (i = 0; i < size; i++) {
                increment_element(arr1, i, 1);
            }
        }
        
        /* Worker partitioned */
        #pragma acc kernels create(arr2[0:size]) worker
        {
            #pragma acc loop worker
            for (i = 0; i < size; i++) {
                arr2[i] = arr1[i] * 3;
            }
        }
    }
}

/* Test with device data environment */
void test_persistent_partitions(int *data, int size, int init_val) {
    /* Enter data with gang partitioning */
    #pragma acc enter data copyin(data[0:size]) gang
    
    /* Multiple compute regions with different partitions */
    #pragma acc parallel present(data) worker
    {
        int i;
        #pragma acc loop worker
        for (i = 0; i < size; i++) {
            data[i] = init_val;
        }
    }
    
    #pragma acc kernels present(data) gang vector
    {
        int i;
        #pragma acc loop gang vector
        for (i = 0; i < size; i++) {
            data[i] += i;
        }
    }
    
    /* Exit data */
    #pragma acc exit data copyout(data[0:size])
}

/* Main test driver */
int main(int argc, char **argv) {
    int *array1, *array2;
    int arr3d[10][20][30];
    int i, j, k;
    
    /* Use argc to create conditional execution paths */
    int test_case = argc > 1 ? atoi(argv[1]) % 8 : 0;
    
    /* Allocate and initialize arrays */
    array1 = (int *)malloc(N * sizeof(int));
    array2 = (int *)malloc(N * sizeof(int));
    
    for (i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = N - i;
    }
    
    /* Initialize 3D array */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 20; j++) {
            for (k = 0; k < 30; k++) {
                arr3d[i][j][k] = 0;
            }
        }
    }
    
    /* Execute different test paths based on input */
    switch (test_case) {
        case 0:
            /* Gang redundant partitioning */
            #pragma acc parallel copy(array1[0:N]) gang
            {
                #pragma acc loop gang
                for (i = 0; i < N; i++) {
                    array1[i] += 1;
                }
            }
            break;
            
        case 1:
            /* Gang partitioned */
            test_partition_combinations(array1, array2, N, 1);
            break;
            
        case 2:
            /* Worker partitioned */
            #pragma acc kernels copy(array1[0:N]) worker
            {
                #pragma acc loop worker
                for (i = 0; i < N; i++) {
                    array1[i] *= 2;
                }
            }
            break;
            
        case 3:
            /* Gang+worker partitioned */
            test_nested_partitions(array1, N);
            break;
            
        case 4:
            /* Vector partitioned */
            #pragma acc parallel copy(array1[0:N]) vector
            {
                #pragma acc loop vector
                for (i = 0; i < N; i++) {
                    array1[i] += array2[i];
                }
            }
            break;
            
        case 5:
            /* Gang+vector partitioned */
            test_partition_combinations(array1, array2, N, 0);
            break;
            
        case 6:
            /* Worker+vector partitioned */
            test_multi_dim_partitions(arr3d);
            break;
            
        case 7:
            /* Fully partitioned (gang+worker+vector) */
            test_persistent_partitions(array1, N, 100);
            break;
            
        default:
            /* Should not reach here, but include for completeness */
            #pragma acc parallel copy(array1[0:N]) gang worker vector
            {
                #pragma acc loop gang worker vector
                for (i = 0; i < N; i++) {
                    array1[i] = 0;
                }
            }
            break;
    }
    
    /* Additional test with conditional directive */
    if (argc > 2) {
        #pragma acc parallel if(argc > 2) copy(array2[0:N]) gang worker
        {
            #pragma acc loop gang worker
            for (i = 0; i < N; i++) {
                array2[i] = array1[i] * array2[i];
            }
        }
    }
    
    /* Validate results (simplified check) */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += array1[i];
    }
    printf("Result checksum: %d\n", sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    
    return 0;
}
