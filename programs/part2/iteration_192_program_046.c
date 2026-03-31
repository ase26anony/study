#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force volatile to prevent optimization */
volatile int select_partition = 0;

/* Struct with array members for testing nested components */
struct DataContainer {
    int grid[64][64];
    float values[128];
    double matrix[32][32];
};

/* Helper to map partition codes to OpenACC partition keywords */
const char* partition_keywords[8] = {
    "",            /* 0: gang redundant (default) */
    "[gang]",      /* 1: gang partitioned */
    "[worker]",    /* 2: worker partitioned */
    "[gang][worker]", /* 3: gang+worker partitioned */
    "[vector]",    /* 4: vector partitioned */
    "[gang][vector]", /* 5: gang+vector partitioned */
    "[worker][vector]", /* 6: worker+vector partitioned */
    "[gang][worker][vector]" /* 7: fully partitioned */
};

int main(int argc, char *argv[]) {
    int i, j, k;
    long checksum = 0;
    
    /* Multi-dimensional arrays for partition testing */
    int md_array[100][100];
    float matrix_3d[50][50][50];
    
    /* Dynamic arrays for pointer-based mappings */
    int *dyn_arr = (int*)malloc(1000 * sizeof(int));
    double *dyn_matrix = (double*)malloc(200 * 200 * sizeof(double));
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Initialize all data */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            md_array[i][j] = i * 100 + j;
        }
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            for (k = 0; k < 50; k++) {
                matrix_3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (i = 0; i < 1000; i++) {
        dyn_arr[i] = i % 100;
    }
    
    for (i = 0; i < 200*200; i++) {
        dyn_matrix[i] = i * 0.5;
    }
    
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            container.grid[i][j] = i * j;
        }
    }
    
    /* Use command-line argument to control partition selection */
    if (argc > 1) {
        select_partition = atoi(argv[1]) % 8;
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* Test 1: Gang redundant (case 0) - default mapping */
    #pragma acc data copy(md_array)
    {
        #pragma acc parallel loop collapse(2)
        for (i = 0; i < 100; i++) {
            for (j = 0; j < 100; j++) {
                md_array[i][j] += 1;
            }
        }
    }
    
    /* Test 2: Gang partitioned (case 1) */
    #pragma acc data copy(md_array[0:50][gang])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 100; j++) {
                md_array[i][j] += 2;
            }
        }
    }
    
    /* Test 3: Worker partitioned (case 2) */
    #pragma acc data copy(md_array[0:100][worker])
    {
        #pragma acc parallel loop worker
        for (i = 0; i < 100; i++) {
            md_array[i][i] += 3;
        }
    }
    
    /* Test 4: Gang+worker partitioned (case 3) - 2D array section */
    #pragma acc data copy(md_array[0:50][gang][worker])
    {
        #pragma acc parallel loop gang worker collapse(2)
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                md_array[i][j] += 4;
            }
        }
    }
    
    /* Test 5: Vector partitioned (case 4) */
    #pragma acc data copy(dyn_arr[0:1000][vector])
    {
        #pragma acc parallel loop vector
        for (i = 0; i < 1000; i++) {
            dyn_arr[i] += 5;
        }
    }
    
    /* Test 6: Gang+vector partitioned (case 5) */
    #pragma acc data copy(dyn_matrix[0:200*200][gang][vector])
    {
        #pragma acc parallel loop gang vector
        for (i = 0; i < 200*200; i += 200) {
            for (j = 0; j < 200; j++) {
                dyn_matrix[i + j] += 6;
            }
        }
    }
    
    /* Test 7: Worker+vector partitioned (case 6) */
    #pragma acc data copy(container.grid[0:64][worker][vector])
    {
        #pragma acc parallel loop worker vector collapse(2)
        for (i = 0; i < 64; i++) {
            for (j = 0; j < 64; j++) {
                container.grid[i][j] += 7;
            }
        }
    }
    
    /* Test 8: Fully partitioned (case 7) - 3D array */
    #pragma acc data copy(matrix_3d[0:50][gang][worker][vector])
    {
        #pragma acc parallel loop gang worker vector collapse(3)
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                for (k = 0; k < 50; k++) {
                    matrix_3d[i][j][k] += 8;
                }
            }
        }
    }
    
    /* Conditional partition selection based on volatile variable */
    /* This creates a single code region where compiler must handle multiple possibilities */
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (select_partition == ptype) {
            /* Use different partition types based on the condition */
            switch (ptype) {
                case 0:
                    #pragma acc data copy(md_array)
                    {
                        #pragma acc parallel loop
                        for (i = 0; i < 10; i++) md_array[i][0] += ptype;
                    }
                    break;
                case 1:
                    #pragma acc data copy(md_array[0:10][gang])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 10; i++) md_array[i][1] += ptype;
                    }
                    break;
                case 2:
                    #pragma acc data copy(md_array[0:10][worker])
                    {
                        #pragma acc parallel loop worker
                        for (i = 0; i < 10; i++) md_array[i][2] += ptype;
                    }
                    break;
                case 3:
                    #pragma acc data copy(md_array[0:10][gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < 10; i++) md_array[i][3] += ptype;
                    }
                    break;
                case 4:
                    #pragma acc data copy(dyn_arr[0:100][vector])
                    {
                        #pragma acc parallel loop vector
                        for (i = 0; i < 100; i++) dyn_arr[i] += ptype;
                    }
                    break;
                case 5:
                    #pragma acc data copy(dyn_arr[0:100][gang][vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (i = 0; i < 100; i++) dyn_arr[i] += ptype;
                    }
                    break;
                case 6:
                    #pragma acc data copy(container.values[0:100][worker][vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (i = 0; i < 100; i++) container.values[i] += ptype;
                    }
                    break;
                case 7:
                    #pragma acc data copy(container.matrix[0:10][gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector collapse(2)
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                container.matrix[i][j] += ptype;
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    /* Test structured data movement with enter/exit data */
    #pragma acc enter data copyin(container.grid[0:32][gang])
    #pragma acc parallel loop gang present(container.grid[0:32][gang])
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            container.grid[i][j] += 9;
        }
    }
    #pragma acc exit data copyout(container.grid[0:32][gang])
    
    /* Nested constructs: data region with internal parallel region */
    #pragma acc data copy(md_array[0:20][gang][worker])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 20; i++) {
            #pragma acc loop worker
            for (j = 0; j < 20; j++) {
                md_array[i][j] += 10;
            }
        }
    }
    
    /* Compute checksum to ensure computations aren't optimized away */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += md_array[i][j];
        }
    }
    
    for (i = 0; i < 1000; i++) {
        checksum += dyn_arr[i];
    }
    
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            checksum += container.grid[i][j];
        }
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    
    return 0;
}
