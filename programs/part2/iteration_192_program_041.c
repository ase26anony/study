#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent compiler optimizations from removing code paths */
volatile int use_partition_type = 0;

/* Struct with array members to test nested component partitioning */
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    double values[200];
};

/* Function to force different partition types based on input */
void test_partition_type(int ptype, int *arr, int size) {
    switch(ptype) {
        case 0: /* gang redundant */
            #pragma acc parallel loop gang copy(arr[0:size])
            for(int i = 0; i < size; i++) {
                arr[i] += 1;
            }
            break;
            
        case 1: /* gang partitioned */
            #pragma acc parallel loop gang copy(arr[0:size][gang])
            for(int i = 0; i < size; i++) {
                arr[i] += 2;
            }
            break;
            
        case 2: /* worker partitioned */
            #pragma acc parallel loop gang worker copy(arr[0:size][worker])
            for(int i = 0; i < size; i++) {
                arr[i] += 3;
            }
            break;
            
        case 3: /* gang+worker partitioned */
            #pragma acc parallel loop gang worker copy(arr[0:size][gang+worker])
            for(int i = 0; i < size; i++) {
                arr[i] += 4;
            }
            break;
            
        case 4: /* vector partitioned */
            #pragma acc parallel loop vector copy(arr[0:size][vector])
            for(int i = 0; i < size; i++) {
                arr[i] += 5;
            }
            break;
            
        case 5: /* gang+vector partitioned */
            #pragma acc parallel loop gang vector copy(arr[0:size][gang+vector])
            for(int i = 0; i < size; i++) {
                arr[i] += 6;
            }
            break;
            
        case 6: /* worker+vector partitioned */
            #pragma acc parallel loop worker vector copy(arr[0:size][worker+vector])
            for(int i = 0; i < size; i++) {
                arr[i] += 7;
            }
            break;
            
        case 7: /* fully partitioned */
            #pragma acc parallel loop gang worker vector copy(arr[0:size][gang+worker+vector])
            for(int i = 0; i < size; i++) {
                arr[i] += 8;
            }
            break;
    }
}

/* Test multi-dimensional array partitioning */
void test_multi_dim_partitions(int arr[100][100]) {
    /* gang partitioned on first dimension */
    #pragma acc data copy(arr[0:50][gang][0:100])
    {
        #pragma acc parallel loop gang
        for(int i = 0; i < 50; i++) {
            for(int j = 0; j < 100; j++) {
                arr[i][j] += i + j;
            }
        }
    }
    
    /* worker partitioned on second dimension */
    #pragma acc data copy(arr[0:100][0:50][worker])
    {
        #pragma acc parallel loop gang worker
        for(int i = 0; i < 100; i++) {
            for(int j = 0; j < 50; j++) {
                arr[i][j] += i * j;
            }
        }
    }
    
    /* gang+worker partitioned (2D section) */
    #pragma acc data copy(arr[0:50][gang][0:50][worker])
    {
        #pragma acc parallel loop gang worker
        for(int i = 0; i < 50; i++) {
            for(int j = 0; j < 50; j++) {
                arr[i][j] += i - j;
            }
        }
    }
    
    /* vector partitioned on second dimension */
    #pragma acc data copy(arr[0:100][0:100][vector])
    {
        #pragma acc parallel loop vector
        for(int i = 0; i < 100; i++) {
            for(int j = 0; j < 100; j++) {
                arr[i][j] += j;
            }
        }
    }
    
    /* gang+worker+vector partitioned (3D would trigger fully partitioned) */
    int arr3d[50][50][50];
    #pragma acc data copy(arr3d[0:50][gang][0:50][worker][0:50][vector])
    {
        #pragma acc parallel loop gang worker vector collapse(3)
        for(int i = 0; i < 50; i++) {
            for(int j = 0; j < 50; j++) {
                for(int k = 0; k < 50; k++) {
                    arr3d[i][j][k] = i + j + k;
                }
            }
        }
    }
}

/* Test dynamic data with pointer-based mappings */
void test_dynamic_partitions() {
    int N = 1000;
    int *dyn_arr = (int*)malloc(N * sizeof(int));
    
    /* Initialize array */
    for(int i = 0; i < N; i++) {
        dyn_arr[i] = i;
    }
    
    /* Test different partition types on dynamic data */
    #pragma acc data copy(dyn_arr[0:N][gang])
    {
        #pragma acc parallel loop gang
        for(int i = 0; i < N; i++) {
            dyn_arr[i] += 1;
        }
    }
    
    #pragma acc data copy(dyn_arr[0:N][worker])
    {
        #pragma acc parallel loop gang worker
        for(int i = 0; i < N; i++) {
            dyn_arr[i] += 2;
        }
    }
    
    #pragma acc data copy(dyn_arr[0:N][vector])
    {
        #pragma acc parallel loop vector
        for(int i = 0; i < N; i++) {
            dyn_arr[i] += 3;
        }
    }
    
    #pragma acc data copy(dyn_arr[0:N][gang+worker])
    {
        #pragma acc parallel loop gang worker
        for(int i = 0; i < N; i++) {
            dyn_arr[i] += 4;
        }
    }
    
    free(dyn_arr);
}

/* Test struct with array members */
void test_struct_partitions(struct DataContainer *container) {
    /* Map struct members with different partition types */
    #pragma acc data copy(container->matrix[0:50][gang][0:50][worker], \
                          container->vector[0:1000][vector], \
                          container->values[0:200][gang+vector])
    {
        #pragma acc parallel loop gang worker
        for(int i = 0; i < 50; i++) {
            for(int j = 0; j < 50; j++) {
                container->matrix[i][j] = i * 100 + j;
            }
        }
        
        #pragma acc parallel loop vector
        for(int i = 0; i < 1000; i++) {
            container->vector[i] = i * 2;
        }
        
        #pragma acc parallel loop gang vector
        for(int i = 0; i < 200; i++) {
            container->values[i] = i * 3.14;
        }
    }
}

/* Test enter/exit data with partitions */
void test_structured_data_movement() {
    int base[500];
    
    /* Initialize */
    for(int i = 0; i < 500; i++) {
        base[i] = i;
    }
    
    /* Enter data with gang partition */
    #pragma acc enter data copyin(base[0:500][gang])
    
    /* Use the data in a parallel region */
    #pragma acc parallel loop gang present(base[0:500][gang])
    for(int i = 0; i < 500; i++) {
        base[i] += 10;
    }
    
    /* Exit data */
    #pragma acc exit data copyout(base[0:500][gang])
}

/* Test conditional partition selection */
void test_conditional_partitions(int *arr, int size) {
    /* Array mapping partition codes to keywords (for illustration) */
    const char* partition_keywords[] = {
        "",
        "[gang]",
        "[worker]", 
        "[gang+worker]",
        "[vector]",
        "[gang+vector]",
        "[worker+vector]",
        "[gang+worker+vector]"
    };
    
    /* Loop through all partition types, using volatile to prevent dead code elimination */
    for(int ptype = 0; ptype < 8; ++ptype) {
        if(use_partition_type == ptype) {  /* Volatile ensures all paths are considered */
            switch(ptype) {
                case 0:
                    #pragma acc parallel loop gang copy(arr[0:size])
                    for(int i = 0; i < size; i++) arr[i] += ptype;
                    break;
                case 1:
                    #pragma acc parallel loop gang copy(arr[0:size][gang])
                    for(int i = 0; i < size; i++) arr[i] += ptype;
                    break;
                case 2:
                    #pragma acc parallel loop gang worker copy(arr[0:size][worker])
                    for(int i = 0; i < size; i++) arr[i] += ptype;
                    break;
                case 3:
                    #pragma acc parallel loop gang worker copy(arr[0:size][gang+worker])
                    for(int i = 0; i < size; i++) arr[i] += ptype;
                    break;
                case 4:
                    #pragma acc parallel loop vector copy(arr[0:size][vector])
                    for(int i = 0; i < size; i++) arr[i] += ptype;
                    break;
                case 5:
                    #pragma acc parallel loop gang vector copy(arr[0:size][gang+vector])
                    for(int i = 0; i < size; i++) arr[i] += ptype;
                    break;
                case 6:
                    #pragma acc parallel loop worker vector copy(arr[0:size][worker+vector])
                    for(int i = 0; i < size; i++) arr[i] += ptype;
                    break;
                case 7:
                    #pragma acc parallel loop gang worker vector copy(arr[0:size][gang+worker+vector])
                    for(int i = 0; i < size; i++) arr[i] += ptype;
                    break;
            }
        }
    }
}

int main() {
    int arr[1000];
    int md_arr[100][100];
    struct DataContainer container;
    long long checksum = 0;
    
    /* Initialize arrays */
    memset(arr, 0, sizeof(arr));
    memset(md_arr, 0, sizeof(md_arr));
    memset(&container, 0, sizeof(container));
    
    printf("Testing all OpenACC partition types for coverage...\n");
    
    /* 1. Test all basic partition types (0-7) */
    for(int i = 0; i < 8; i++) {
        test_partition_type(i, arr, 1000);
    }
    
    /* 2. Test multi-dimensional array partitions */
    test_multi_dim_partitions(md_arr);
    
    /* 3. Test dynamic data partitions */
    test_dynamic_partitions();
    
    /* 4. Test struct with array members */
    test_struct_partitions(&container);
    
    /* 5. Test structured data movement */
    test_structured_data_movement();
    
    /* 6. Test conditional partitions */
    test_conditional_partitions(arr, 1000);
    
    /* Compute checksum to ensure computations aren't optimized away */
    for(int i = 0; i < 1000; i++) {
        checksum += arr[i];
    }
    
    for(int i = 0; i < 100; i++) {
        for(int j = 0; j < 100; j++) {
            checksum += md_arr[i][j];
        }
    }
    
    for(int i = 0; i < 50; i++) {
        for(int j = 0; j < 50; j++) {
            checksum += container.matrix[i][j];
        }
    }
    
    for(int i = 0; i < 1000; i++) {
        checksum += container.vector[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    printf("All partition types should have been exercised.\n");
    
    return 0;
}
