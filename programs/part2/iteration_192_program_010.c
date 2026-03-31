#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Force compiler to consider all code paths */
volatile int select_partition = 0;

/* Struct with array members for requirement #6 */
struct DataContainer {
    int matrix[10][10][10];
    int vector[1000];
    double grid[50][50];
};

/* Partition mapping for requirement #5 */
const char* partition_map[] = {
    "",           /* 0: gang redundant */
    "[gang]",     /* 1: gang partitioned */
    "[worker]",   /* 2: worker partitioned */
    "[gang][worker]", /* 3: gang+worker partitioned */
    "[vector]",   /* 4: vector partitioned */
    "[gang][vector]", /* 5: gang+vector partitioned */
    "[worker][vector]", /* 6: worker+vector partitioned */
    "[gang][worker][vector]" /* 7: fully partitioned */
};

void test_gang_redundant(int* arr, int n) {
    #pragma acc parallel loop copy(arr[0:n]) /* case 0 */
    for (int i = 0; i < n; i++) {
        arr[i] += 1;
    }
}

void test_gang_partitioned(int* arr, int n) {
    #pragma acc parallel loop gang copy(arr[0:n][gang]) /* case 1 */
    for (int i = 0; i < n; i++) {
        arr[i] += 2;
    }
}

void test_worker_partitioned(int* arr, int n) {
    #pragma acc parallel loop worker copy(arr[0:n][worker]) /* case 2 */
    for (int i = 0; i < n; i++) {
        arr[i] += 3;
    }
}

void test_gang_worker_partitioned(int arr[10][10]) {
    #pragma acc parallel loop gang worker copy(arr[0:10][gang][worker]) /* case 3 */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] += 4;
        }
    }
}

void test_vector_partitioned(int* arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n][vector]) /* case 4 */
    for (int i = 0; i < n; i++) {
        arr[i] += 5;
    }
}

void test_gang_vector_partitioned(int arr[10][10]) {
    #pragma acc parallel loop gang vector copy(arr[0:10][gang][vector]) /* case 5 */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] += 6;
        }
    }
}

void test_worker_vector_partitioned(int arr[10][10]) {
    #pragma acc parallel loop worker vector copy(arr[0:10][worker][vector]) /* case 6 */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] += 7;
        }
    }
}

void test_fully_partitioned(struct DataContainer* s) {
    /* 3D array with full partitioning - case 7 */
    #pragma acc parallel loop gang worker vector \
        copy(s->matrix[0:10][gang][worker][vector])
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                s->matrix[i][j][k] += 8;
            }
        }
    }
}

/* Dynamic data with pointer-based mappings - requirement #3 */
void test_dynamic_partitions(int** dyn_arr, int rows, int cols) {
    /* Allocate and initialize dynamic 2D array */
    *dyn_arr = (int*)malloc(rows * cols * sizeof(int));
    for (int i = 0; i < rows * cols; i++) {
        (*dyn_arr)[i] = i;
    }
    
    /* Map with gang+worker partitioning */
    #pragma acc data copy((*dyn_arr)[0:rows*cols][gang][worker])
    {
        #pragma acc parallel loop gang worker
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                (*dyn_arr)[idx] += 9;
            }
        }
    }
}

/* Nested constructs with enter/exit data - requirement #4 */
void test_nested_data_regions(int* data, int n) {
    /* Enter data with gang partitioning */
    #pragma acc enter data copyin(data[0:n][gang])
    
    /* Parallel region using present data */
    #pragma acc parallel loop gang present(data[0:n][gang])
    for (int i = 0; i < n; i++) {
        data[i] += 10;
    }
    
    /* Exit data */
    #pragma acc exit data copyout(data[0:n])
}

/* Conditional partition selection - requirement #5 */
void test_conditional_partitions(int* arr, int n) {
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (select_partition == ptype) {
            /* Use different partition types based on volatile variable */
            switch (ptype) {
                case 0:
                    #pragma acc parallel loop copy(arr[0:n])
                    for (int i = 0; i < n; i++) arr[i] += 100;
                    break;
                case 1:
                    #pragma acc parallel loop gang copy(arr[0:n][gang])
                    for (int i = 0; i < n; i++) arr[i] += 200;
                    break;
                case 2:
                    #pragma acc parallel loop worker copy(arr[0:n][worker])
                    for (int i = 0; i < n; i++) arr[i] += 300;
                    break;
                case 3:
                    #pragma acc parallel loop gang worker copy(arr[0:n][gang][worker])
                    for (int i = 0; i < n; i++) arr[i] += 400;
                    break;
                case 4:
                    #pragma acc parallel loop vector copy(arr[0:n][vector])
                    for (int i = 0; i < n; i++) arr[i] += 500;
                    break;
                case 5:
                    #pragma acc parallel loop gang vector copy(arr[0:n][gang][vector])
                    for (int i = 0; i < n; i++) arr[i] += 600;
                    break;
                case 6:
                    #pragma acc parallel loop worker vector copy(arr[0:n][worker][vector])
                    for (int i = 0; i < n; i++) arr[i] += 700;
                    break;
                case 7:
                    #pragma acc parallel loop gang worker vector copy(arr[0:n][gang][worker][vector])
                    for (int i = 0; i < n; i++) arr[i] += 800;
                    break;
            }
        }
    }
}

int main() {
    int checksum = 0;
    
    /* Test 1: Basic arrays with different partition types */
    int array1[100];
    int array2[10][10];
    int array3[10][10][10];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) array1[i] = i;
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            array2[i][j] = i * 10 + j;
    
    /* Test all partition cases explicitly */
    test_gang_redundant(array1, 100);           /* case 0 */
    test_gang_partitioned(array1, 100);         /* case 1 */
    test_worker_partitioned(array1, 100);       /* case 2 */
    test_gang_worker_partitioned(array2);       /* case 3 */
    test_vector_partitioned(array1, 100);       /* case 4 */
    test_gang_vector_partitioned(array2);       /* case 5 */
    test_worker_vector_partitioned(array2);     /* case 6 */
    
    /* Test 2: Struct with array members */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    test_fully_partitioned(&container);         /* case 7 */
    
    /* Test 3: Dynamic data */
    int* dynamic_array = NULL;
    test_dynamic_partitions(&dynamic_array, 20, 20);
    
    /* Test 4: Nested data regions */
    int nested_data[50];
    for (int i = 0; i < 50; i++) nested_data[i] = i;
    test_nested_data_regions(nested_data, 50);
    
    /* Test 5: Conditional partitions */
    int cond_array[100];
    for (int i = 0; i < 100; i++) cond_array[i] = i;
    test_conditional_partitions(cond_array, 100);
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < 100; i++) checksum += array1[i];
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            checksum += array2[i][j];
    for (int i = 0; i < 50; i++) checksum += nested_data[i];
    for (int i = 0; i < 100; i++) checksum += cond_array[i];
    
    if (dynamic_array) {
        for (int i = 0; i < 400; i++) checksum += dynamic_array[i];
        free(dynamic_array);
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Attempt to trigger default case (illegal partition) */
    /* Note: This might cause compilation error, but we include it
       to potentially exercise error handling paths */
    #ifdef TEST_ILLEGAL
    int illegal_array[10];
    /* This should generate an illegal partition code if the compiler
       tries to parse it */
    #pragma acc parallel copy(illegal_array[0:10][invalid])
    {
        /* Empty */
    }
    #endif
    
    return 0;
}

#ifdef __cplusplus
}
#endif
