#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Force compiler to consider all code paths */
volatile int select_partition = 0;
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;

#ifdef __cplusplus
}
#endif

#define N 1024
#define M 512
#define P 256

struct DataContainer {
    int multi_arr[10][20][30];
    float dyn_member[N];
    double matrix[M][M];
};

/* Array mapping partition codes to keywords */
const char* partition_keywords[] = {
    "",            /* 0: gang redundant */
    "[gang]",      /* 1: gang partitioned */
    "[worker]",    /* 2: worker partitioned */
    "[gang+worker]", /* 3: gang+worker partitioned */
    "[vector]",    /* 4: vector partitioned */
    "[gang+vector]", /* 5: gang+vector partitioned */
    "[worker+vector]", /* 6: worker+vector partitioned */
    "[gang+worker+vector]" /* 7: fully partitioned */
};

void test_gang_redundant(int* arr) {
    #pragma acc parallel loop gang copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] += 1;
    }
}

void test_gang_partitioned(int* arr) {
    #pragma acc parallel loop gang copy(arr[0:N][gang])
    for (int i = 0; i < N; i++) {
        arr[i] += 2;
    }
}

void test_worker_partitioned(int* arr) {
    #pragma acc parallel loop worker copy(arr[0:N][worker])
    for (int i = 0; i < N; i++) {
        arr[i] += 3;
    }
}

void test_gang_worker_partitioned(int arr2d[M][M]) {
    #pragma acc parallel loop gang worker copy(arr2d[0:M][0:M][gang+worker])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] += i + j;
        }
    }
}

void test_vector_partitioned(float* arr) {
    #pragma acc parallel loop vector copy(arr[0:N][vector])
    for (int i = 0; i < N; i++) {
        arr[i] *= 1.5f;
    }
}

void test_gang_vector_partitioned(double arr3d[P][P][P]) {
    #pragma acc parallel loop gang vector copy(arr3d[0:P/2][0:P/2][0:P/2][gang+vector])
    for (int i = 0; i < P/2; i++) {
        for (int j = 0; j < P/2; j++) {
            for (int k = 0; k < P/2; k++) {
                arr3d[i][j][k] += 0.1;
            }
        }
    }
}

void test_worker_vector_partitioned(struct DataContainer* s) {
    #pragma acc parallel loop worker vector copy(s->dyn_member[0:N][worker+vector])
    for (int i = 0; i < N; i++) {
        s->dyn_member[i] += i * 0.01f;
    }
}

void test_fully_partitioned(struct DataContainer* s) {
    /* Map different dimensions with different partitions */
    #pragma acc data copy(s->multi_arr[0:5][0:10][0:15][gang+worker+vector])
    {
        #pragma acc parallel loop gang worker vector
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                for (int k = 0; k < 15; k++) {
                    s->multi_arr[i][j][k] = i * 100 + j * 10 + k;
                }
            }
        }
    }
}

void test_combined_regions(int* base_arr, float* dyn_arr, double matrix[M][M]) {
    /* Test enter/exit data with partitions */
    #pragma acc enter data copyin(base_arr[0:N/2][gang])
    
    #pragma acc parallel loop gang present(base_arr[0:N/2][gang])
    for (int i = 0; i < N/2; i++) {
        base_arr[i] *= 2;
    }
    
    #pragma acc exit data copyout(base_arr[0:N/2][gang])
    
    /* Nested data region */
    #pragma acc data copy(dyn_arr[0:N][vector])
    {
        #pragma acc parallel loop vector
        for (int i = 0; i < N; i++) {
            dyn_arr[i] = i * 0.5f;
        }
        
        /* Inner region with different partition */
        #pragma acc data copy(matrix[0:M/4][0:M/4][worker])
        {
            #pragma acc parallel loop worker
            for (int i = 0; i < M/4; i++) {
                for (int j = 0; j < M/4; j++) {
                    matrix[i][j] = i * j * 0.01;
                }
            }
        }
    }
}

void test_conditional_partitions(int* arr, int partition_code) {
    /* Use volatile to prevent optimization */
    if (use_gang && partition_code == 1) {
        #pragma acc parallel loop gang copy(arr[0:100][gang])
        for (int i = 0; i < 100; i++) {
            arr[i] += partition_code;
        }
    }
    
    if (use_worker && partition_code == 2) {
        #pragma acc parallel loop worker copy(arr[100:200][worker])
        for (int i = 100; i < 300; i++) {
            arr[i] += partition_code;
        }
    }
    
    if (use_vector && partition_code == 4) {
        #pragma acc parallel loop vector copy(arr[300:400][vector])
        for (int i = 300; i < 700; i++) {
            arr[i] += partition_code;
        }
    }
}

int main() {
    int* base_array = (int*)malloc(N * sizeof(int));
    float* dyn_array = (float*)malloc(N * sizeof(float));
    double (*matrix)[M] = (double(*)[M])malloc(M * M * sizeof(double));
    double (*arr3d)[P][P] = (double(*)[P][P])malloc(P * P * P * sizeof(double));
    
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        base_array[i] = i;
        dyn_array[i] = (float)i;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = 0.0;
        }
    }
    
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < P; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = 0.0;
            }
        }
    }
    
    printf("Testing all OpenACC partition types...\n");
    
    /* Test each partition type explicitly */
    test_gang_redundant(base_array);           /* Case 0 */
    test_gang_partitioned(base_array);         /* Case 1 */
    test_worker_partitioned(base_array);       /* Case 2 */
    test_gang_worker_partitioned(matrix);      /* Case 3 */
    test_vector_partitioned(dyn_array);        /* Case 4 */
    test_gang_vector_partitioned(arr3d);       /* Case 5 */
    test_worker_vector_partitioned(&container); /* Case 6 */
    test_fully_partitioned(&container);        /* Case 7 */
    
    /* Test combined constructs */
    test_combined_regions(base_array, dyn_array, matrix);
    
    /* Test conditional partitions - loop through all codes */
    for (int pcode = 0; pcode < 8; pcode++) {
        if (select_partition == pcode) {
            test_conditional_partitions(base_array, pcode);
        }
    }
    
    /* Compute checksums to ensure computations happen */
    long long int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    for (int i = 0; i < N; i++) {
        int_sum += base_array[i];
        float_sum += dyn_array[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            double_sum += matrix[i][j];
        }
    }
    
    printf("Checksums - Int: %lld, Float: %.2f, Double: %.2f\n", 
           int_sum, float_sum, double_sum);
    
    /* Cleanup */
    free(base_array);
    free(dyn_array);
    free(matrix);
    free(arr3d);
    
    return 0;
}
