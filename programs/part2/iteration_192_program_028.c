#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent compiler optimizations that might eliminate code paths */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members to test nested component partitioning */
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
    /* Multi-dimensional arrays for partition testing */
    int array_2d[100][100];
    int array_3d[50][50][50];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            array_2d[i][j] = i + j;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            for (int k = 0; k < 50; k++) {
                array_3d[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Dynamic allocated memory for pointer-based mappings */
    int *dyn_arr = (int*)malloc(1000 * sizeof(int));
    for (int i = 0; i < 1000; i++) {
        dyn_arr[i] = i % 100;
    }
    
    /* Struct instance */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    for (int i = 0; i < 1000; i++) {
        container.vector[i] = i * 2;
    }
    
    int checksum = 0;
    
    /* ========== CASE 0: gang redundant (default mapping) ========== */
    if (use_gang) {
        #pragma acc data copy(array_2d)
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    array_2d[i][j] += 1;
                }
            }
        }
        checksum += compute_checksum(&array_2d[0][0], 100*100);
    }
    
    /* ========== CASE 1: gang partitioned ========== */
    if (use_gang) {
        #pragma acc data copy(array_2d[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    array_2d[i][j] += 2;
                }
            }
        }
        checksum += compute_checksum(&array_2d[0][0], 100*100);
    }
    
    /* ========== CASE 2: worker partitioned ========== */
    if (use_worker) {
        #pragma acc data copy(array_2d[0:100][worker])
        {
            #pragma acc parallel loop worker
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    array_2d[i][j] += 3;
                }
            }
        }
        checksum += compute_checksum(&array_2d[0][0], 100*100);
    }
    
    /* ========== CASE 3: gang+worker partitioned ========== */
    if (use_combined) {
        /* Using 2D array with gang on first dim, worker on second */
        #pragma acc data copy(array_2d[0:100][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    array_2d[i][j] += 4;
                }
            }
        }
        checksum += compute_checksum(&array_2d[0][0], 100*100);
    }
    
    /* ========== CASE 4: vector partitioned ========== */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:1000][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 1000; i++) {
                dyn_arr[i] += 5;
            }
        }
        checksum += compute_checksum(dyn_arr, 1000);
    }
    
    /* ========== CASE 5: gang+vector partitioned ========== */
    if (use_combined) {
        /* Using struct member with gang+vector partition */
        #pragma acc data copy(container.vector[0:1000][gang+vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < 1000; i++) {
                container.vector[i] += 6;
            }
        }
        checksum += compute_checksum(container.vector, 1000);
    }
    
    /* ========== CASE 6: worker+vector partitioned ========== */
    if (use_combined) {
        /* Using dynamic array with worker+vector partition */
        #pragma acc data copy(dyn_arr[0:1000][worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < 1000; i++) {
                dyn_arr[i] += 7;
            }
        }
        checksum += compute_checksum(dyn_arr, 1000);
    }
    
    /* ========== CASE 7: fully partitioned (gang+worker+vector) ========== */
    if (use_combined) {
        /* Using 3D array with all three partition types */
        #pragma acc data copy(array_3d[0:50][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    for (int k = 0; k < 50; k++) {
                        array_3d[i][j][k] += 8;
                    }
                }
            }
        }
        checksum += compute_checksum(&array_3d[0][0][0], 50*50*50);
    }
    
    /* ========== Testing enter/exit data with partitions ========== */
    {
        int temp_arr[500];
        for (int i = 0; i < 500; i++) temp_arr[i] = i;
        
        /* Enter data with gang partition */
        #pragma acc enter data copyin(temp_arr[0:500][gang])
        
        #pragma acc parallel loop gang present(temp_arr[gang])
        for (int i = 0; i < 500; i++) {
            temp_arr[i] *= 2;
        }
        
        #pragma acc exit data copyout(temp_arr[0:500][gang])
        checksum += compute_checksum(temp_arr, 500);
    }
    
    /* ========== Conditional partition selection ========== */
    /* Array mapping partition codes to partition specifiers */
    const char* partition_specs[] = {
        "",
        "[gang]",
        "[worker]", 
        "[gang][worker]",
        "[vector]",
        "[gang+vector]",
        "[worker+vector]",
        "[gang][worker][vector]"
    };
    
    /* Use different partition types based on volatile conditions */
    int test_arr[200];
    for (int i = 0; i < 200; i++) test_arr[i] = i;
    
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (ptype % 2 == (use_gang ? 1 : 0)) {
            /* Create a unique map clause for each partition type */
            switch (ptype) {
                case 0:
                    #pragma acc data copy(test_arr)
                    {
                        #pragma acc parallel loop
                        for (int i = 0; i < 200; i++) test_arr[i] += 1;
                    }
                    break;
                case 1:
                    #pragma acc data copy(test_arr[0:200][gang])
                    {
                        #pragma acc parallel loop gang
                        for (int i = 0; i < 200; i++) test_arr[i] += 2;
                    }
                    break;
                case 2:
                    #pragma acc data copy(test_arr[0:200][worker])
                    {
                        #pragma acc parallel loop worker
                        for (int i = 0; i < 200; i++) test_arr[i] += 3;
                    }
                    break;
                case 3:
                    #pragma acc data copy(test_arr[0:200][gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (int i = 0; i < 200; i++) test_arr[i] += 4;
                    }
                    break;
                case 4:
                    #pragma acc data copy(test_arr[0:200][vector])
                    {
                        #pragma acc parallel loop vector
                        for (int i = 0; i < 200; i++) test_arr[i] += 5;
                    }
                    break;
                case 5:
                    #pragma acc data copy(test_arr[0:200][gang+vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (int i = 0; i < 200; i++) test_arr[i] += 6;
                    }
                    break;
                case 6:
                    #pragma acc data copy(test_arr[0:200][worker+vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (int i = 0; i < 200; i++) test_arr[i] += 7;
                    }
                    break;
                case 7:
                    #pragma acc data copy(test_arr[0:200][gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (int i = 0; i < 200; i++) test_arr[i] += 8;
                    }
                    break;
            }
        }
    }
    checksum += compute_checksum(test_arr, 200);
    
    /* ========== Struct with multiple partitioned arrays ========== */
    {
        #pragma acc data copy(container.matrix[0:50][gang], container.values[0:200][vector])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 50; i++) {
                #pragma acc loop worker
                for (int j = 0; j < 50; j++) {
                    container.matrix[i][j] = i * j;
                }
            }
            
            #pragma acc parallel loop vector
            for (int i = 0; i < 200; i++) {
                container.values[i] = i * 0.5;
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(dyn_arr);
    return 0;
}
