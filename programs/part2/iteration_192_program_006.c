#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent dead code elimination */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members for requirement #6 */
struct DataContainer {
    int matrix[50][50];
    int linear[1000];
    double values[200];
};

/* Function to compute checksum */
int compute_checksum(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    int i, j;
    
    /* Multi-dimensional arrays for requirement #2 */
    int md_array[100][100];
    int md_array2[50][50][50];
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            md_array[i][j] = i * 100 + j;
        }
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            for (int k = 0; k < 50; k++) {
                md_array2[i][j][k] = i * 2500 + j * 50 + k;
            }
        }
    }
    
    /* Struct instance for requirement #6 */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    
    /* Dynamic data for requirement #3 */
    int *dyn_arr = (int*)malloc(1000 * sizeof(int));
    for (i = 0; i < 1000; i++) {
        dyn_arr[i] = i;
    }
    
    int total_checksum = 0;
    
    /* CASE 0: Gang redundant (default mapping) */
    if (use_gang) {
        #pragma acc data copy(md_array)
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    md_array[i][j] += 1;
                }
            }
        }
        total_checksum += compute_checksum(&md_array[0][0], 100*100);
    }
    
    /* CASE 1: Gang partitioned */
    if (use_gang) {
        #pragma acc data copy(md_array[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    md_array[i][j] += 2;
                }
            }
        }
        total_checksum += compute_checksum(&md_array[0][0], 100*100);
    }
    
    /* CASE 2: Worker partitioned */
    if (use_worker) {
        #pragma acc data copy(md_array[0:100][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 100; i++) {
                #pragma acc loop worker
                for (j = 0; j < 100; j++) {
                    md_array[i][j] += 3;
                }
            }
        }
        total_checksum += compute_checksum(&md_array[0][0], 100*100);
    }
    
    /* CASE 3: Gang+worker partitioned */
    if (use_combined) {
        #pragma acc data copy(md_array2[0:50][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 50; i++) {
                #pragma acc loop worker
                for (j = 0; j < 50; j++) {
                    for (int k = 0; k < 50; k++) {
                        md_array2[i][j][k] += 4;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&md_array2[0][0][0], 50*50*50);
    }
    
    /* CASE 4: Vector partitioned */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:1000][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 1000; i++) {
                dyn_arr[i] += 5;
            }
        }
        total_checksum += compute_checksum(dyn_arr, 1000);
    }
    
    /* CASE 5: Gang+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(md_array[0:100][gang+vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < 100; i++) {
                #pragma acc loop vector
                for (j = 0; j < 100; j++) {
                    md_array[i][j] += 6;
                }
            }
        }
        total_checksum += compute_checksum(&md_array[0][0], 100*100);
    }
    
    /* CASE 6: Worker+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(md_array[0:100][worker+vector])
        {
            #pragma acc parallel loop gang worker vector
            for (i = 0; i < 100; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 100; j++) {
                    md_array[i][j] += 7;
                }
            }
        }
        total_checksum += compute_checksum(&md_array[0][0], 100*100);
    }
    
    /* CASE 7: Fully partitioned (gang+worker+vector) */
    if (use_combined) {
        #pragma acc data copy(md_array2[0:50][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (i = 0; i < 50; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 50; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 50; k++) {
                        md_array2[i][j][k] += 8;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&md_array2[0][0][0], 50*50*50);
    }
    
    /* Struct with partitioned array members - requirement #6 */
    #pragma acc data copy(container.matrix[gang], container.linear[worker], container.values[vector])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < 50; j++) {
                container.matrix[i][j] = i + j;
            }
        }
        
        #pragma acc parallel loop worker
        for (i = 0; i < 1000; i++) {
            container.linear[i] = i * 2;
        }
        
        #pragma acc parallel loop vector
        for (i = 0; i < 200; i++) {
            container.values[i] = i * 0.5;
        }
    }
    
    /* Nested constructs for requirement #4 */
    #pragma acc enter data copyin(md_array[0:50][gang])
    #pragma acc parallel loop present(md_array[0:50][gang])
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 100; j++) {
            md_array[i][j] += 9;
        }
    }
    #pragma acc exit data copyout(md_array[0:50][gang])
    
    total_checksum += compute_checksum(&md_array[0][0], 100*100);
    
    /* Conditional partition selection - requirement #5 */
    const char* partition_map[8] = {
        "",           /* 0: gang redundant */
        "[gang]",     /* 1: gang partitioned */
        "[worker]",   /* 2: worker partitioned */
        "[gang][worker]", /* 3: gang+worker partitioned */
        "[vector]",   /* 4: vector partitioned */
        "[gang+vector]", /* 5: gang+vector partitioned */
        "[worker+vector]", /* 6: worker+vector partitioned */
        "[gang][worker][vector]" /* 7: fully partitioned */
    };
    
    int test_array[100];
    for (i = 0; i < 100; i++) test_array[i] = i;
    
    /* This loop structure forces the compiler to consider all partition types */
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (ptype % 2 == 0) {  /* Simple condition to vary execution path */
            /* We can't use string concatenation in pragma, so we use switch */
            switch(ptype) {
                case 0:
                    #pragma acc data copy(test_array)
                    {
                        #pragma acc parallel loop
                        for (i = 0; i < 100; i++) test_array[i] += 1;
                    }
                    break;
                case 1:
                    #pragma acc data copy(test_array[0:100][gang])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 100; i++) test_array[i] += 2;
                    }
                    break;
                case 2:
                    #pragma acc data copy(test_array[0:100][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < 100; i++) test_array[i] += 3;
                    }
                    break;
                case 3:
                    #pragma acc data copy(test_array[0:100][gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < 100; i++) test_array[i] += 4;
                    }
                    break;
                case 4:
                    #pragma acc data copy(test_array[0:100][vector])
                    {
                        #pragma acc parallel loop vector
                        for (i = 0; i < 100; i++) test_array[i] += 5;
                    }
                    break;
                case 5:
                    #pragma acc data copy(test_array[0:100][gang+vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (i = 0; i < 100; i++) test_array[i] += 6;
                    }
                    break;
                case 6:
                    #pragma acc data copy(test_array[0:100][worker+vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (i = 0; i < 100; i++) test_array[i] += 7;
                    }
                    break;
                case 7:
                    #pragma acc data copy(test_array[0:100][gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (i = 0; i < 100; i++) test_array[i] += 8;
                    }
                    break;
            }
        }
    }
    
    total_checksum += compute_checksum(test_array, 100);
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Clean up */
    free(dyn_arr);
    
    return 0;
}
