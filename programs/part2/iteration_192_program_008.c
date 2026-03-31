#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent dead code elimination */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members for testing nested components */
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    double values[200];
};

/* Function to compute checksum */
int compute_checksum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Multi-dimensional arrays for complex partition testing */
    int multi_arr[100][100];
    int arr3d[50][50][50];
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Dynamic arrays for pointer-based mappings */
    int *dyn_arr = (int*)malloc(1000 * sizeof(int));
    double *dyn_matrix = (double*)malloc(100 * 100 * sizeof(double));
    
    if (!dyn_arr || !dyn_matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    memset(multi_arr, 0, sizeof(multi_arr));
    memset(arr3d, 0, sizeof(arr3d));
    memset(&container, 0, sizeof(container));
    
    for (int i = 0; i < 1000; i++) {
        dyn_arr[i] = i % 100;
    }
    
    int total_checksum = 0;
    
    /* Test Case 0: Gang redundant (default mapping) */
    if (use_gang) {
        #pragma acc data copy(multi_arr)
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    multi_arr[i][j] += 1;
                }
            }
        }
        total_checksum += compute_checksum(&multi_arr[0][0], 100*100);
    }
    
    /* Test Case 1: Gang partitioned */
    if (use_gang) {
        #pragma acc data copy(multi_arr[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    multi_arr[i][j] += 2;
                }
            }
        }
    }
    
    /* Test Case 2: Worker partitioned */
    if (use_worker) {
        #pragma acc data copy(container.vector[0:1000][worker])
        {
            #pragma acc parallel loop worker
            for (int i = 0; i < 1000; i++) {
                container.vector[i] = i * 2;
            }
        }
        total_checksum += compute_checksum(container.vector, 1000);
    }
    
    /* Test Case 3: Gang+worker partitioned */
    if (use_combined) {
        #pragma acc data copy(arr3d[0:50][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    for (int k = 0; k < 50; k++) {
                        arr3d[i][j][k] = i + j + k;
                    }
                }
            }
        }
    }
    
    /* Test Case 4: Vector partitioned */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:1000][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 1000; i++) {
                dyn_arr[i] *= 3;
            }
        }
        total_checksum += compute_checksum(dyn_arr, 1000);
    }
    
    /* Test Case 5: Gang+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(container.matrix[0:50][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    container.matrix[i][j] = i * j;
                }
            }
        }
        total_checksum += compute_checksum(&container.matrix[0][0], 50*50);
    }
    
    /* Test Case 6: Worker+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(dyn_matrix[0:100*100][worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < 100*100; i++) {
                dyn_matrix[i] = i * 0.5;
            }
        }
    }
    
    /* Test Case 7: Fully partitioned (gang+worker+vector) */
    if (use_gang && use_worker && use_vector) {
        #pragma acc data copy(arr3d[0:50][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    for (int k = 0; k < 50; k++) {
                        arr3d[i][j][k] += 1;
                    }
                }
            }
        }
    }
    
    /* Test nested constructs with partitioned mappings */
    {
        int nested_arr[200][200];
        memset(nested_arr, 0, sizeof(nested_arr));
        
        /* Enter data with gang partition */
        #pragma acc enter data copyin(nested_arr[0:200][gang])
        
        /* Parallel region using present data */
        #pragma acc parallel loop present(nested_arr[0:200][gang])
        for (int i = 0; i < 200; i++) {
            for (int j = 0; j < 200; j++) {
                nested_arr[i][j] = i + j;
            }
        }
        
        /* Exit data */
        #pragma acc exit data copyout(nested_arr[0:200][gang])
        
        total_checksum += compute_checksum(&nested_arr[0][0], 200*200);
    }
    
    /* Conditional partition selection using command-line argument */
    if (argc > 1) {
        int ptype = atoi(argv[1]) % 8;
        
        /* Array mapping partition codes to keywords */
        const char* partition_keywords[] = {
            "",           /* 0: gang redundant */
            "[gang]",     /* 1: gang partitioned */
            "[worker]",   /* 2: worker partitioned */
            "[gang][worker]", /* 3: gang+worker partitioned */
            "[vector]",   /* 4: vector partitioned */
            "[gang][vector]", /* 5: gang+vector partitioned */
            "[worker+vector]", /* 6: worker+vector partitioned */
            "[gang][worker][vector]" /* 7: fully partitioned */
        };
        
        int test_arr[100];
        
        /* This creates a single code region where compiler must handle
           multiple possible partition codes through constant propagation */
        switch (ptype) {
            case 0:
                #pragma acc data copy(test_arr)
                #pragma acc parallel loop
                for (int i = 0; i < 100; i++) test_arr[i] = 1;
                break;
            case 1:
                #pragma acc data copy(test_arr[0:100][gang])
                #pragma acc parallel loop gang
                for (int i = 0; i < 100; i++) test_arr[i] = 2;
                break;
            case 2:
                #pragma acc data copy(test_arr[0:100][worker])
                #pragma acc parallel loop worker
                for (int i = 0; i < 100; i++) test_arr[i] = 3;
                break;
            case 3:
                #pragma acc data copy(test_arr[0:100][gang][worker])
                #pragma acc parallel loop gang worker
                for (int i = 0; i < 100; i++) test_arr[i] = 4;
                break;
            case 4:
                #pragma acc data copy(test_arr[0:100][vector])
                #pragma acc parallel loop vector
                for (int i = 0; i < 100; i++) test_arr[i] = 5;
                break;
            case 5:
                #pragma acc data copy(test_arr[0:100][gang][vector])
                #pragma acc parallel loop gang vector
                for (int i = 0; i < 100; i++) test_arr[i] = 6;
                break;
            case 6:
                #pragma acc data copy(test_arr[0:100][worker+vector])
                #pragma acc parallel loop worker vector
                for (int i = 0; i < 100; i++) test_arr[i] = 7;
                break;
            case 7:
                #pragma acc data copy(test_arr[0:100][gang][worker][vector])
                #pragma acc parallel loop gang worker vector
                for (int i = 0; i < 100; i++) test_arr[i] = 8;
                break;
        }
        
        total_checksum += compute_checksum(test_arr, 100);
    }
    
    /* Final checksum output to prevent optimization */
    printf("Total checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    
    return 0;
}
