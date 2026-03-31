#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 100
#define P 100

// Struct with array members for requirement #6
struct DataContainer {
    int array1[N][M];
    int array2[N][M];
    int *dyn_array;
};

// Volatile variable to prevent optimization (requirement #5)
volatile int select_partition = 0;

// Array mapping partition codes to keywords
const char* partition_map[] = {
    "",           // 0: gang redundant (default)
    "[gang]",     // 1: gang partitioned
    "[worker]",   // 2: worker partitioned
    "[gang][worker]",  // 3: gang+worker partitioned
    "[vector]",   // 4: vector partitioned
    "[gang][vector]",  // 5: gang+vector partitioned
    "[worker][vector]", // 6: worker+vector partitioned
    "[gang][worker][vector]" // 7: fully partitioned
};

void test_gang_redundant() {
    int arr[N][M];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * M + j;
        }
    }
    
    // Case 0: gang redundant (default mapping)
    #pragma acc parallel loop gang copy(arr)
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker vector
        for (int j = 0; j < M; j++) {
            arr[i][j] += 1;
        }
    }
    
    // Verify
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    printf("Gang redundant sum: %d\n", sum);
}

void test_gang_partitioned() {
    int arr[N][M];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * M + j;
        }
    }
    
    // Case 1: gang partitioned
    #pragma acc parallel loop gang copy(arr[gang])
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker vector
        for (int j = 0; j < M; j++) {
            arr[i][j] += 2;
        }
    }
    
    printf("Gang partitioned completed\n");
}

void test_worker_partitioned() {
    int arr[N][M];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * M + j;
        }
    }
    
    // Case 2: worker partitioned
    #pragma acc parallel loop gang copy(arr[worker])
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker vector
        for (int j = 0; j < M; j++) {
            arr[i][j] += 3;
        }
    }
    
    printf("Worker partitioned completed\n");
}

void test_gang_worker_partitioned() {
    int arr[N][M][P];
    
    // Initialize 3D array (requirement #2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    // Case 3: gang+worker partitioned on 3D array
    #pragma acc data copy(arr[0:N][gang][worker])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 4;
                }
            }
        }
    }
    
    printf("Gang+worker partitioned completed\n");
}

void test_vector_partitioned() {
    int *dyn_arr = (int*)malloc(N * M * sizeof(int));  // Dynamic data (requirement #3)
    
    // Initialize
    for (int i = 0; i < N * M; i++) {
        dyn_arr[i] = i;
    }
    
    // Case 4: vector partitioned with dynamic array
    #pragma acc parallel loop vector copy(dyn_arr[0:N*M][vector])
    for (int i = 0; i < N * M; i++) {
        dyn_arr[i] += 5;
    }
    
    free(dyn_arr);
    printf("Vector partitioned completed\n");
}

void test_gang_vector_partitioned() {
    struct DataContainer container;  // Struct with arrays (requirement #6)
    
    // Initialize struct arrays
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            container.array1[i][j] = i * M + j;
            container.array2[i][j] = (i * M + j) * 2;
        }
    }
    
    // Case 5: gang+vector partitioned on struct member
    #pragma acc data copy(container.array1[gang][vector])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                container.array1[i][j] += 6;
            }
        }
    }
    
    printf("Gang+vector partitioned completed\n");
}

void test_worker_vector_partitioned() {
    int arr[N][M];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * M + j;
        }
    }
    
    // Case 6: worker+vector partitioned
    #pragma acc data copy(arr[worker][vector])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] += 7;
            }
        }
    }
    
    printf("Worker+vector partitioned completed\n");
}

void test_fully_partitioned() {
    int arr[N][M][P];
    
    // Initialize 3D array
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    // Case 7: fully partitioned (gang+worker+vector)
    #pragma acc data copy(arr[gang][worker][vector])
    {
        #pragma acc parallel loop gang
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
    
    printf("Fully partitioned completed\n");
}

void test_conditional_partitions() {
    int arr[N][M];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * M + j;
        }
    }
    
    // Requirement #5: Conditional partition selection
    // Using volatile to prevent optimization
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (select_partition == ptype) {
            // This creates different code paths for the compiler to analyze
            #pragma acc data copy(arr)
            {
                #pragma acc parallel loop gang
                for (int i = 0; i < N; i++) {
                    #pragma acc loop worker vector
                    for (int j = 0; j < M; j++) {
                        arr[i][j] += ptype + 10;
                    }
                }
            }
        }
    }
    
    printf("Conditional partitions completed\n");
}

void test_enter_exit_data() {
    int *base = (int*)malloc(N * M * sizeof(int));
    
    // Initialize
    for (int i = 0; i < N * M; i++) {
        base[i] = i;
    }
    
    // Requirement #4: enter/exit data with partitions
    #pragma acc enter data copyin(base[0:N*M][gang])
    
    #pragma acc parallel loop gang present(base[gang])
    for (int i = 0; i < N * M; i++) {
        base[i] += 20;
    }
    
    #pragma acc exit data copyout(base[0:N*M][gang])
    
    free(base);
    printf("Enter/exit data with partitions completed\n");
}

int main() {
    printf("Testing all partition types for OpenACC neutering coverage\n\n");
    
    // Test each partition type explicitly
    test_gang_redundant();        // Case 0
    test_gang_partitioned();      // Case 1
    test_worker_partitioned();    // Case 2
    test_gang_worker_partitioned(); // Case 3
    test_vector_partitioned();    // Case 4
    test_gang_vector_partitioned(); // Case 5
    test_worker_vector_partitioned(); // Case 6
    test_fully_partitioned();     // Case 7
    
    // Additional tests to ensure all code paths are exercised
    test_conditional_partitions();
    test_enter_exit_data();
    
    printf("\nAll partition tests completed\n");
    
    // Create observable side effect (requirement #5)
    int final_arr[10];
    #pragma acc parallel loop copy(final_arr)
    for (int i = 0; i < 10; i++) {
        final_arr[i] = i * 2;
    }
    
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += final_arr[i];
    }
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
