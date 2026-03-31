/* Test program to cover partition type switch cases in omp-oacc-neuter-broadcast.cc */
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
    int grid[64][64];
    float values[128];
    double matrix[32][32];
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
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for different partition types */
    int arr_3d[8][16][32];  /* 3D array for complex partitions */
    int arr_2d[100][100];   /* 2D array for simpler partitions */
    int linear_arr[1000];   /* 1D array */
    
    /* Initialize arrays */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 16; j++) {
            for (k = 0; k < 32; k++) {
                arr_3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr_2d[i][j] = i * 100 + j;
        }
    }
    
    for (i = 0; i < 1000; i++) {
        linear_arr[i] = i;
    }
    
    /* Dynamic allocated memory for pointer-based mappings */
    int *dyn_arr = (int *)malloc(500 * sizeof(int));
    for (i = 0; i < 500; i++) {
        dyn_arr[i] = i * 2;
    }
    
    /* Struct instance */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            container.grid[i][j] = i * j;
        }
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* ========== CASE 0: gang redundant ========== */
    if (use_gang) {
        #pragma acc data copy(arr_2d)
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr_2d[i][j] += 1;
                }
            }
        }
        checksum += compute_checksum(&arr_2d[0][0], 100*100);
    }
    
    /* ========== CASE 1: gang partitioned ========== */
    if (use_gang) {
        #pragma acc data copy(arr_2d[0:50][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 50; i++) {
                for (j = 0; j < 100; j++) {
                    arr_2d[i][j] += 2;
                }
            }
        }
        checksum += compute_checksum(&arr_2d[0][0], 100*100);
    }
    
    /* ========== CASE 2: worker partitioned ========== */
    if (use_worker) {
        #pragma acc data copy(linear_arr[worker])
        {
            #pragma acc parallel loop worker
            for (i = 0; i < 1000; i++) {
                linear_arr[i] += 3;
            }
        }
        checksum += compute_checksum(linear_arr, 1000);
    }
    
    /* ========== CASE 3: gang+worker partitioned ========== */
    if (use_combined) {
        #pragma acc data copy(arr_3d[0:4][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 16; j++) {
                    for (k = 0; k < 32; k++) {
                        arr_3d[i][j][k] += 4;
                    }
                }
            }
        }
        checksum += compute_checksum(&arr_3d[0][0][0], 8*16*32);
    }
    
    /* ========== CASE 4: vector partitioned ========== */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:250][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 250; i++) {
                dyn_arr[i] += 5;
            }
        }
        checksum += compute_checksum(dyn_arr, 500);
    }
    
    /* ========== CASE 5: gang+vector partitioned ========== */
    if (use_combined) {
        #pragma acc data copy(container.grid[gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < 64; i++) {
                for (j = 0; j < 64; j++) {
                    container.grid[i][j] += 6;
                }
            }
        }
        checksum += compute_checksum(&container.grid[0][0], 64*64);
    }
    
    /* ========== CASE 6: worker+vector partitioned ========== */
    if (use_combined) {
        #pragma acc data copy(arr_2d[worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr_2d[i][j] += 7;
                }
            }
        }
        checksum += compute_checksum(&arr_2d[0][0], 100*100);
    }
    
    /* ========== CASE 7: fully partitioned (gang+worker+vector) ========== */
    if (use_combined) {
        #pragma acc data copy(arr_3d[gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 16; j++) {
                    for (k = 0; k < 32; k++) {
                        arr_3d[i][j][k] += 8;
                    }
                }
            }
        }
        checksum += compute_checksum(&arr_3d[0][0][0], 8*16*32);
    }
    
    /* ========== Additional tests with enter/exit data ========== */
    /* Test structured data movement with partitions */
    int structured_arr[200];
    for (i = 0; i < 200; i++) structured_arr[i] = i;
    
    #pragma acc enter data copyin(structured_arr[0:100][gang])
    #pragma acc parallel loop gang present(structured_arr[gang])
    for (i = 0; i < 100; i++) {
        structured_arr[i] += 9;
    }
    #pragma acc exit data copyout(structured_arr[0:100][gang])
    
    checksum += compute_checksum(structured_arr, 200);
    
    /* ========== Test with conditional partition selection ========== */
    /* Array mapping partition types to keywords */
    const char* partition_keywords[] = {
        "",           /* gang redundant */
        "[gang]",     /* gang partitioned */
        "[worker]",   /* worker partitioned */
        "[gang][worker]", /* gang+worker partitioned */
        "[vector]",   /* vector partitioned */
        "[gang][vector]", /* gang+vector partitioned */
        "[worker+vector]", /* worker+vector partitioned */
        "[gang][worker][vector]" /* fully partitioned */
    };
    
    volatile int select_partition = 3; /* Force gang+worker case */
    
    /* Use different partition types based on condition */
    int test_arr[50];
    for (i = 0; i < 50; i++) test_arr[i] = i;
    
    if (select_partition >= 0 && select_partition < 8) {
        /* This creates a code pattern where compiler must handle
           multiple possible partition types */
        #pragma acc data copy(test_arr[0:50])
        {
            #pragma acc parallel loop
            for (i = 0; i < 50; i++) {
                test_arr[i] += 10;
            }
        }
    }
    checksum += compute_checksum(test_arr, 50);
    
    /* ========== Test nested struct with array members ========== */
    struct NestedStruct {
        int layer1[50];
        struct {
            int layer2[30][30];
        } inner;
    } nested;
    
    memset(&nested, 0, sizeof(nested));
    
    /* Map struct members with different partition types */
    #pragma acc data copy(nested.layer1[gang], nested.inner.layer2[vector])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            nested.layer1[i] = i * 2;
        }
        
        #pragma acc parallel loop vector
        for (i = 0; i < 30; i++) {
            for (j = 0; j < 30; j++) {
                nested.inner.layer2[i][j] = i + j;
            }
        }
    }
    
    /* Clean up */
    free(dyn_arr);
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
