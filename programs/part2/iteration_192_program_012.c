#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 100
#define P 100

volatile int select_partition = 0;

struct DataContainer {
    int array1[N][M];
    int array2[N][M];
    double values[P];
};

void test_gang_redundant(int arr[N][M]) {
    #pragma acc parallel loop gang copy(arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] += 1;
        }
    }
}

void test_gang_partitioned(int arr[N][M]) {
    #pragma acc parallel loop gang copy(arr[0:N][gang])
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker vector
        for (int j = 0; j < M; j++) {
            arr[i][j] += 2;
        }
    }
}

void test_worker_partitioned(int arr[N][M]) {
    #pragma acc parallel loop gang copy(arr[0:N][worker])
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker vector
        for (int j = 0; j < M; j++) {
            arr[i][j] += 3;
        }
    }
}

void test_gang_worker_partitioned(int arr[N][M]) {
    #pragma acc parallel loop gang copy(arr[0:N][gang][worker])
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            #pragma acc loop vector
            for (int k = 0; k < 10; k++) {
                arr[i][j] += 4;
            }
        }
    }
}

void test_vector_partitioned(int *arr) {
    #pragma acc parallel loop gang worker copy(arr[0:N*M][vector])
    for (int i = 0; i < N*M; i++) {
        arr[i] += 5;
    }
}

void test_gang_vector_partitioned(int arr[N][M]) {
    #pragma acc parallel loop gang copy(arr[0:N][gang][vector])
    for (int i = 0; i < N; i++) {
        #pragma acc loop vector
        for (int j = 0; j < M; j++) {
            arr[i][j] += 6;
        }
    }
}

void test_worker_vector_partitioned(int arr[N][M]) {
    #pragma acc parallel loop gang copy(arr[0:N][worker][vector])
    for (int i = 0; i < N; i++) {
        #pragma acc loop vector
        for (int j = 0; j < M; j++) {
            arr[i][j] += 7;
        }
    }
}

void test_fully_partitioned(int arr[N][M][P]) {
    #pragma acc parallel loop gang copy(arr[0:N][gang][worker][vector])
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            #pragma acc loop vector
            for (int k = 0; k < P; k++) {
                arr[i][j][k] += 8;
            }
        }
    }
}

void test_struct_partitions(struct DataContainer *s) {
    // Test gang partitioned on struct member
    #pragma acc data copy(s->array1[0:N][gang], s->array2[0:N][worker])
    {
        #pragma acc parallel loop gang present(s->array1[0:N][gang])
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                s->array1[i][j] += 9;
            }
        }
        
        #pragma acc parallel loop gang present(s->array2[0:N][worker])
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                s->array2[i][j] += 10;
            }
        }
    }
}

void test_dynamic_memory() {
    int *dyn_arr = (int*)malloc(N * M * sizeof(int));
    memset(dyn_arr, 0, N * M * sizeof(int));
    
    // Test worker+vector partitioned on dynamic memory
    #pragma acc parallel loop gang copy(dyn_arr[0:N*M][worker+vector])
    for (int i = 0; i < N*M; i++) {
        dyn_arr[i] += 11;
    }
    
    free(dyn_arr);
}

void test_enter_exit_data() {
    int base[N][M];
    memset(base, 0, sizeof(base));
    
    // Test gang partitioned with enter/exit data
    #pragma acc enter data copyin(base[0:N][gang])
    
    #pragma acc parallel loop gang present(base[0:N][gang])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            base[i][j] += 12;
        }
    }
    
    #pragma acc exit data copyout(base[0:N][gang])
}

int main() {
    int array1[N][M];
    int array2[N][M];
    int array3[N][M][P];
    int *flat_array = (int*)malloc(N * M * sizeof(int));
    struct DataContainer container;
    
    // Initialize arrays
    memset(array1, 0, sizeof(array1));
    memset(array2, 0, sizeof(array2));
    memset(array3, 0, sizeof(array3));
    memset(flat_array, 0, N * M * sizeof(int));
    memset(&container, 0, sizeof(container));
    
    // Use volatile to prevent optimization
    volatile int partition_selector = 0;
    
    // Test all partition types through conditional execution
    // This ensures compiler considers all possible paths
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (partition_selector == ptype) {
            switch(ptype) {
                case 0:  // gang redundant
                    test_gang_redundant(array1);
                    break;
                case 1:  // gang partitioned
                    test_gang_partitioned(array1);
                    break;
                case 2:  // worker partitioned
                    test_worker_partitioned(array1);
                    break;
                case 3:  // gang+worker partitioned
                    test_gang_worker_partitioned(array1);
                    break;
                case 4:  // vector partitioned
                    test_vector_partitioned(flat_array);
                    break;
                case 5:  // gang+vector partitioned
                    test_gang_vector_partitioned(array2);
                    break;
                case 6:  // worker+vector partitioned
                    test_worker_vector_partitioned(array2);
                    break;
                case 7:  // fully partitioned
                    test_fully_partitioned(array3);
                    break;
            }
        }
    }
    
    // Additional tests for complex scenarios
    test_struct_partitions(&container);
    test_dynamic_memory();
    test_enter_exit_data();
    
    // Compute checksum to ensure computations aren't optimized away
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += array1[i][j] + array2[i][j] + container.array1[i][j] + container.array2[i][j];
        }
    }
    
    for (int i = 0; i < N*M; i++) {
        checksum += flat_array[i];
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                checksum += array3[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    free(flat_array);
    return 0;
}
