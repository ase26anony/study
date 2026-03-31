/* Test program to cover partition type switch cases in omp-oacc-neuter-broadcast.cc
   Lines 335-343: gang redundant (0) through fully partitioned (7) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent dead code elimination */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Partition mapping array for conditional tests */
const char* partition_map[8] = {
    "",          /* 0: gang redundant */
    "[gang]",    /* 1: gang partitioned */
    "[worker]",  /* 2: worker partitioned */
    "[gang][worker]", /* 3: gang+worker partitioned */
    "[vector]",  /* 4: vector partitioned */
    "[gang][vector]", /* 5: gang+vector partitioned */
    "[worker][vector]", /* 6: worker+vector partitioned */
    "[gang][worker][vector]" /* 7: fully partitioned */
};

/* Struct with array members for complex mapping tests */
struct DataContainer {
    int matrix[50][50];
    int linear[1000];
    double values[200];
};

int main() {
    int i, j, k;
    int checksum = 0;
    
    /* 1. Multi-dimensional arrays for different partition types */
    int arr2d[100][100];
    int arr3d[30][30][30];
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] = i + j;
        }
    }
    
    for (i = 0; i < 30; i++) {
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 30; k++) {
                arr3d[i][j][k] = i * j * k;
            }
        }
    }
    
    /* 2. Dynamic allocated memory */
    int *dyn_arr = (int*)malloc(500 * sizeof(int));
    for (i = 0; i < 500; i++) {
        dyn_arr[i] = i * 2;
    }
    
    /* 3. Struct with array members */
    struct DataContainer container;
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            container.matrix[i][j] = i - j;
        }
    }
    for (i = 0; i < 1000; i++) {
        container.linear[i] = i % 100;
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* CASE 0: Gang redundant (default mapping) */
    if (use_gang) {
        #pragma acc data copy(arr2d)
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 1;
                }
            }
        }
    }
    
    /* CASE 1: Gang partitioned */
    if (use_gang) {
        #pragma acc data copy(arr2d[0:50][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 50; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] *= 2;
                }
            }
        }
    }
    
    /* CASE 2: Worker partitioned */
    if (use_worker) {
        #pragma acc data copy(arr2d[0:100][worker])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                #pragma acc loop worker
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] -= j;
                }
            }
        }
    }
    
    /* CASE 3: Gang+Worker partitioned */
    if (use_combined) {
        #pragma acc data copy(arr3d[0:20][gang][worker])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 20; i++) {
                #pragma acc loop worker
                for (j = 0; j < 30; j++) {
                    #pragma acc loop vector
                    for (k = 0; k < 30; k++) {
                        arr3d[i][j][k] += i + j + k;
                    }
                }
            }
        }
    }
    
    /* CASE 4: Vector partitioned */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:500][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 500; i++) {
                dyn_arr[i] += 5;
            }
        }
    }
    
    /* CASE 5: Gang+Vector partitioned */
    if (use_combined) {
        #pragma acc data copy(container.matrix[0:30][gang][vector])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 30; i++) {
                #pragma acc loop vector
                for (j = 0; j < 50; j++) {
                    container.matrix[i][j] *= 3;
                }
            }
        }
    }
    
    /* CASE 6: Worker+Vector partitioned */
    if (use_combined) {
        #pragma acc data copy(container.linear[0:500][worker][vector])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 5; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 100; j++) {
                    int idx = i * 100 + j;
                    container.linear[idx] += idx % 7;
                }
            }
        }
    }
    
    /* CASE 7: Fully partitioned (gang+worker+vector) */
    if (use_combined) {
        #pragma acc data copy(arr3d[0:10][gang][worker][vector])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 10; i++) {
                #pragma acc loop worker
                for (j = 0; j < 20; j++) {
                    #pragma acc loop vector
                    for (k = 0; k < 20; k++) {
                        arr3d[i][j][k] = arr3d[i][j][k] / 2;
                    }
                }
            }
        }
    }
    
    /* 4. Conditional partition selection using volatile variables */
    /* This creates code paths where compiler must handle multiple partition types */
    for (int iter = 0; iter < 3; iter++) {
        volatile int selector = iter % 3;
        
        if (selector == 0 && use_gang) {
            #pragma acc data copy(arr2d[0:30][gang])
            {
                #pragma acc parallel loop gang
                for (i = 0; i < 30; i++) {
                    #pragma acc loop worker vector
                    for (j = 0; j < 100; j++) {
                        arr2d[i][j] += 10;
                    }
                }
            }
        } else if (selector == 1 && use_worker) {
            #pragma acc data copy(arr2d[30:40][worker])
            {
                #pragma acc parallel loop gang
                for (i = 30; i < 70; i++) {
                    #pragma acc loop worker
                    for (j = 0; j < 100; j++) {
                        arr2d[i][j] -= 5;
                    }
                }
            }
        } else if (selector == 2 && use_vector) {
            #pragma acc data copy(arr2d[70:30][vector])
            {
                #pragma acc parallel loop vector
                for (i = 70; i < 100; i++) {
                    for (j = 0; j < 100; j++) {
                        arr2d[i][j] *= 1.5;
                    }
                }
            }
        }
    }
    
    /* 5. Nested constructs with enter/exit data */
    /* Gang partitioned enter data */
    #pragma acc enter data copyin(arr2d[0:50][gang])
    
    #pragma acc parallel loop gang present(arr2d[0:50][gang])
    for (i = 0; i < 50; i++) {
        #pragma acc loop worker vector
        for (j = 0; j < 100; j++) {
            arr2d[i][j] += 1000;
        }
    }
    
    #pragma acc exit data copyout(arr2d[0:50][gang])
    
    /* 6. Combined mapping in single directive */
    /* This may trigger complex partition analysis */
    #pragma acc data copy(arr2d[0:25][gang], arr2d[25:25][worker], \
                         arr2d[50:25][vector], arr2d[75:25][gang][worker])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 100; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < 100; j++) {
                arr2d[i][j] %= 1000;
            }
        }
    }
    
    /* Compute checksum for observable side effect */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    for (i = 0; i < 30; i++) {
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 30; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    for (i = 0; i < 500; i++) {
        checksum += dyn_arr[i];
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            checksum += container.matrix[i][j];
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed - all partition types should have been processed.\n");
    
    free(dyn_arr);
    return 0;
}
