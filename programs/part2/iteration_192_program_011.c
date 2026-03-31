#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Force compiler to not optimize away variables */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members for testing nested components */
struct DataContainer {
    int grid[100][100];
    int linear[1000];
    double matrix[50][50];
};

/* Function to prevent dead code elimination */
void __attribute__((noinline)) use_result(int val) {
    volatile int sink = val;
    (void)sink;
}

int main(int argc, char *argv[]) {
    int i, j, k;
    long checksum = 0;
    
    /* Multi-dimensional arrays for complex partition testing */
    int arr3d[10][20][30];
    int arr2d[100][100];
    struct DataContainer container;
    
    /* Dynamic arrays for pointer-based mappings */
    int *dyn_arr = (int*)malloc(1000 * sizeof(int));
    double *dyn_matrix = (double*)malloc(200 * 200 * sizeof(double));
    
    /* Initialize all arrays */
    memset(arr3d, 0, sizeof(arr3d));
    memset(arr2d, 0, sizeof(arr2d));
    memset(&container, 0, sizeof(container));
    memset(dyn_arr, 0, 1000 * sizeof(int));
    memset(dyn_matrix, 0, 200 * 200 * sizeof(double));
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* ========== CASE 0: gang redundant ========== */
    if (use_gang) {
        #pragma acc data copy(arr2d)
        {
            #pragma acc parallel loop gang collapse(2)
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += i + j;
                }
            }
        }
        checksum += arr2d[0][0];
    }
    
    /* ========== CASE 1: gang partitioned ========== */
    if (use_gang) {
        #pragma acc data copy(arr2d[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += i * j;
                }
            }
        }
        checksum += arr2d[1][1];
    }
    
    /* ========== CASE 2: worker partitioned ========== */
    if (use_worker) {
        #pragma acc data copy(arr2d[0:100][worker])
        {
            #pragma acc parallel loop worker
            for (i = 0; i < 100; i++) {
                arr2d[i][i] += i * 2;
            }
        }
        checksum += arr2d[2][2];
    }
    
    /* ========== CASE 3: gang+worker partitioned ========== */
    if (use_combined) {
        #pragma acc data copy(arr3d[0:10][gang][worker])
        {
            #pragma acc parallel loop gang worker collapse(2)
            for (i = 0; i < 10; i++) {
                for (j = 0; j < 20; j++) {
                    for (k = 0; k < 30; k++) {
                        arr3d[i][j][k] = i * 100 + j * 10 + k;
                    }
                }
            }
        }
        checksum += arr3d[0][0][0];
    }
    
    /* ========== CASE 4: vector partitioned ========== */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:1000][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 1000; i++) {
                dyn_arr[i] += i % 256;
            }
        }
        checksum += dyn_arr[0];
    }
    
    /* ========== CASE 5: gang+vector partitioned ========== */
    if (use_combined) {
        #pragma acc data copy(container.grid[0:100][gang][vector])
        {
            #pragma acc parallel loop gang vector collapse(2)
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    container.grid[i][j] = i - j;
                }
            }
        }
        checksum += container.grid[0][0];
    }
    
    /* ========== CASE 6: worker+vector partitioned ========== */
    if (use_combined) {
        #pragma acc data copy(container.linear[0:1000][worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (i = 0; i < 1000; i++) {
                container.linear[i] += i * 3;
            }
        }
        checksum += container.linear[0];
    }
    
    /* ========== CASE 7: fully partitioned ========== */
    if (use_gang && use_worker && use_vector) {
        #pragma acc data copy(arr3d[0:10][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (i = 0; i < 10; i++) {
                for (j = 0; j < 20; j++) {
                    for (k = 0; k < 30; k++) {
                        arr3d[i][j][k] += 1;
                    }
                }
            }
        }
        checksum += arr3d[9][19][29];
    }
    
    /* ========== Additional tests with enter/exit data ========== */
    /* Test structured data movement with partitions */
    #pragma acc enter data copyin(container.matrix[0:50][gang])
    #pragma acc parallel loop gang present(container.matrix[0:50][gang])
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            container.matrix[i][j] = i * 0.5 + j * 0.25;
        }
    }
    #pragma acc exit data copyout(container.matrix[0:50][gang])
    checksum += (int)container.matrix[0][0];
    
    /* ========== Test nested constructs ========== */
    #pragma acc data copy(dyn_matrix[0:200*200][gang+worker])
    {
        #pragma acc kernels
        {
            #pragma acc loop gang worker
            for (i = 0; i < 200; i++) {
                #pragma acc loop vector
                for (j = 0; j < 200; j++) {
                    dyn_matrix[i*200 + j] = (i + j) * 0.1;
                }
            }
        }
    }
    checksum += (int)dyn_matrix[0];
    
    /* ========== Conditional partition selection ========== */
    /* This creates a single code region where compiler must handle multiple possibilities */
    for (int iter = 0; iter < 3; iter++) {
        volatile int selector = iter;
        
        if (selector == 0) {
            #pragma acc data copy(arr2d[0:50][gang])
            {
                #pragma acc parallel loop gang
                for (i = 0; i < 50; i++) {
                    arr2d[i][i] += 100;
                }
            }
        } else if (selector == 1) {
            #pragma acc data copy(arr2d[0:50][worker])
            {
                #pragma acc parallel loop worker
                for (i = 0; i < 50; i++) {
                    arr2d[i][i] += 200;
                }
            }
        } else {
            #pragma acc data copy(arr2d[0:50][vector])
            {
                #pragma acc parallel loop vector
                for (i = 0; i < 50; i++) {
                    arr2d[i][i] += 300;
                }
            }
        }
    }
    checksum += arr2d[49][49];
    
    /* ========== Test invalid/edge cases ========== */
    /* Try to trigger default case if possible */
    /* Note: Actual invalid partition specifiers might cause compilation errors,
       but we include this to potentially exercise error paths */
    {
        int temp[10];
        #pragma acc data copy(temp[0:10])
        {
            #pragma acc parallel loop
            for (i = 0; i < 10; i++) {
                temp[i] = i;
            }
        }
        checksum += temp[0];
    }
    
    /* Final computation and output */
    printf("Final checksum: %ld\n", checksum);
    use_result(checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
