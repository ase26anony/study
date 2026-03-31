#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
#define VOLATILE volatile
#else
#define VOLATILE
#endif

/* Struct with array members for testing nested partition analysis */
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    double values[200];
};

/* Function to prevent dead code elimination */
void use_result(int val) {
    VOLATILE int dummy = val;
    (void)dummy;
}

int main(int argc, char *argv[]) {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for different partition types */
    int arr3d[20][30][40];
    int arr2d[100][100];
    int arr1d[1000];
    
    /* Dynamic arrays for pointer-based mappings */
    int *dyn_arr = (int*)malloc(500 * sizeof(int));
    double *dyn_matrix = (double*)malloc(100 * 100 * sizeof(double));
    
    /* Struct with arrays */
    struct DataContainer container;
    
    /* Initialize all arrays */
    for (i = 0; i < 1000; i++) {
        if (i < 1000) arr1d[i] = i % 100;
        if (dyn_arr && i < 500) dyn_arr[i] = i * 2;
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] = i + j;
        }
    }
    
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 40; k++) {
                arr3d[i][j][k] = i * 1000 + j * 100 + k;
            }
        }
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            dyn_matrix[i * 100 + j] = (double)(i + j) / 100.0;
        }
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            container.matrix[i][j] = i * j;
        }
    }
    
    /* Use volatile to prevent constant propagation and dead code elimination */
    VOLATILE int partition_selector = 0;
    if (argc > 1) {
        partition_selector = atoi(argv[1]);
    }
    
    /* ============================================
       Test Case 0: Gang Redundant (default mapping)
       ============================================ */
    #pragma acc data copy(arr1d[0:1000])
    {
        #pragma acc parallel loop
        for (i = 0; i < 1000; i++) {
            arr1d[i] += 1;
        }
    }
    
    /* ============================================
       Test Case 1: Gang Partitioned
       ============================================ */
    if (partition_selector != 999) { /* Always true, prevents dead code elimination */
        #pragma acc data copy(arr2d[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += i;
                }
            }
        }
    }
    
    /* ============================================
       Test Case 2: Worker Partitioned
       ============================================ */
    #pragma acc data copy(arr1d[0:1000][worker])
    {
        #pragma acc parallel loop worker
        for (i = 0; i < 1000; i++) {
            arr1d[i] += 2;
        }
    }
    
    /* ============================================
       Test Case 3: Gang+Worker Partitioned
       ============================================ */
    #pragma acc data copy(arr2d[0:100][gang][worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < 100; i++) {
            #pragma acc loop worker
            for (j = 0; j < 100; j++) {
                arr2d[i][j] += j;
            }
        }
    }
    
    /* ============================================
       Test Case 4: Vector Partitioned
       ============================================ */
    if (dyn_arr) {
        #pragma acc data copy(dyn_arr[0:500][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 500; i++) {
                dyn_arr[i] += 3;
            }
        }
    }
    
    /* ============================================
       Test Case 5: Gang+Vector Partitioned
       ============================================ */
    #pragma acc data copy(arr2d[0:100][gang][vector])
    {
        #pragma acc parallel loop gang vector
        for (i = 0; i < 100; i++) {
            #pragma acc loop vector
            for (j = 0; j < 100; j++) {
                arr2d[i][j] += 4;
            }
        }
    }
    
    /* ============================================
       Test Case 6: Worker+Vector Partitioned
       ============================================ */
    #pragma acc data copy(arr1d[0:1000][worker][vector])
    {
        #pragma acc parallel loop worker vector
        for (i = 0; i < 1000; i++) {
            arr1d[i] += 5;
        }
    }
    
    /* ============================================
       Test Case 7: Fully Partitioned (gang+worker+vector)
       ============================================ */
    #pragma acc data copy(arr3d[0:20][gang][worker][vector])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 20; i++) {
            #pragma acc loop worker
            for (j = 0; j < 30; j++) {
                #pragma acc loop vector
                for (k = 0; k < 40; k++) {
                    arr3d[i][j][k] += 6;
                }
            }
        }
    }
    
    /* ============================================
       Test struct with array members
       ============================================ */
    #pragma acc data copy(container.matrix[0:50][gang], container.vector[0:1000][worker])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            #pragma acc loop worker
            for (j = 0; j < 50; j++) {
                container.matrix[i][j] += 7;
            }
        }
        
        #pragma acc parallel loop worker
        for (i = 0; i < 1000; i++) {
            container.vector[i] += 8;
        }
    }
    
    /* ============================================
       Test nested constructs with enter/exit data
       ============================================ */
    if (dyn_matrix) {
        #pragma acc enter data copyin(dyn_matrix[0:10000][gang][vector])
        
        #pragma acc parallel loop gang vector present(dyn_matrix[0:10000][gang][vector])
        for (i = 0; i < 10000; i++) {
            dyn_matrix[i] *= 2.0;
        }
        
        #pragma acc exit data copyout(dyn_matrix[0:10000][gang][vector])
    }
    
    /* ============================================
       Test conditional partition selection
       This creates a single code region where compiler
       must handle multiple possible partition codes
       ============================================ */
    for (int ptype = 0; ptype < 8; ++ptype) {
        VOLATILE int cond = (partition_selector == ptype);
        if (cond) {
            switch (ptype) {
                case 0:
                    #pragma acc data copy(arr1d[0:1000])
                    break;
                case 1:
                    #pragma acc data copy(arr1d[0:1000][gang])
                    break;
                case 2:
                    #pragma acc data copy(arr1d[0:1000][worker])
                    break;
                case 3:
                    #pragma acc data copy(arr1d[0:1000][gang][worker])
                    break;
                case 4:
                    #pragma acc data copy(arr1d[0:1000][vector])
                    break;
                case 5:
                    #pragma acc data copy(arr1d[0:1000][gang][vector])
                    break;
                case 6:
                    #pragma acc data copy(arr1d[0:1000][worker][vector])
                    break;
                case 7:
                    #pragma acc data copy(arr1d[0:1000][gang][worker][vector])
                    break;
            }
            {
                #pragma acc parallel loop
                for (i = 0; i < 1000; i++) {
                    arr1d[i] += ptype;
                }
            }
        }
    }
    
    /* Compute checksum to ensure computations aren't optimized away */
    for (i = 0; i < 1000; i++) {
        checksum += arr1d[i];
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 30; j++) {
            for (k = 0; k < 40; k++) {
                checksum += arr3d[i][j][k] % 1000;
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    use_result(checksum);
    
    /* Cleanup */
    if (dyn_arr) free(dyn_arr);
    if (dyn_matrix) free(dyn_matrix);
    
    return 0;
}
