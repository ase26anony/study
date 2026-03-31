/* test_omp_acc_partitions.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 64
#define M 32
#define P 16

/* Function prototypes with routine directives */
#pragma acc routine vec
void increment_vector(int *arr, int n);

#pragma acc routine gang
void process_gang(int *arr, int n);

#pragma acc routine worker
void process_worker(int *arr, int n);

/* Test functions for different partition combinations */
void test_gang_redundant(int data[N][M][P]);
void test_gang_partitioned(int data[N][M][P]);
void test_worker_partitioned(int data[N][M][P]);
void test_gang_worker_partitioned(int data[N][M][P]);
void test_vector_partitioned(int data[N][M][P]);
void test_gang_vector_partitioned(int data[N][M][P]);
void test_worker_vector_partitioned(int data[N][M][P]);
void test_fully_partitioned(int data[N][M][P]);
void test_nested_regions(int data[N][M][P]);
void test_conditional_offloading(int data[N][M][P], int condition);

/* Implement routine functions */
#pragma acc routine vec
void increment_vector(int *arr, int n) {
    #pragma acc loop vector
    for (int i = 0; i < n; i++) {
        arr[i] += 1;
    }
}

#pragma acc routine gang
void process_gang(int *arr, int n) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        arr[i] *= 2;
    }
}

#pragma acc routine worker
void process_worker(int *arr, int n) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        arr[i] += 10;
    }
}

/* Test 0: Gang redundant */
void test_gang_redundant(int data[N][M][P]) {
    printf("Testing gang redundant partitioning...\n");
    
    #pragma acc parallel copy(data[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    data[i][j][k] += i + j + k;
                }
            }
        }
    }
}

/* Test 1: Gang partitioned */
void test_gang_partitioned(int data[N][M][P]) {
    printf("Testing gang partitioned...\n");
    
    #pragma acc kernels create(data[0:N][0:M][0:P]) gang(static:2)
    {
        #pragma acc loop gang(static:2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    data[i][j][k] *= 2;
                }
            }
        }
    }
}

/* Test 2: Worker partitioned */
void test_worker_partitioned(int data[N][M][P]) {
    printf("Testing worker partitioned...\n");
    
    #pragma acc parallel copy(data[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    data[i][j][k] -= 5;
                }
            }
        }
    }
}

/* Test 3: Gang+worker partitioned */
void test_gang_worker_partitioned(int data[N][M][P]) {
    printf("Testing gang+worker partitioned...\n");
    
    #pragma acc kernels copy(data[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    data[i][j][k] = data[i][j][k] * 3 / 2;
                }
            }
        }
    }
}

/* Test 4: Vector partitioned */
void test_vector_partitioned(int data[N][M][P]) {
    printf("Testing vector partitioned...\n");
    
    #pragma acc parallel copy(data[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    data[i][j][k] += 100;
                }
            }
        }
    }
}

/* Test 5: Gang+vector partitioned */
void test_gang_vector_partitioned(int data[N][M][P]) {
    printf("Testing gang+vector partitioned...\n");
    
    #pragma acc kernels copy(data[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    data[i][j][k] = data[i][j][k] << 1;
                }
            }
        }
    }
}

/* Test 6: Worker+vector partitioned */
void test_worker_vector_partitioned(int data[N][M][P]) {
    printf("Testing worker+vector partitioned...\n");
    
    #pragma acc parallel copy(data[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    data[i][j][k] |= 0xFF;
                }
            }
        }
    }
}

/* Test 7: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int data[N][M][P]) {
    printf("Testing fully partitioned...\n");
    
    #pragma acc kernels copy(data[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    data[i][j][k] = ~data[i][j][k];
                }
            }
        }
    }
}

/* Test nested regions with different partition types */
void test_nested_regions(int data[N][M][P]) {
    printf("Testing nested regions...\n");
    
    /* First region with gang partitioning */
    #pragma acc parallel copy(data[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            process_gang(&data[i][0][0], M * P);
        }
    }
    
    /* Second region with worker partitioning */
    #pragma acc kernels copy(data[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            process_worker(&data[i][0][0], M * P);
        }
    }
    
    /* Third region with vector partitioning */
    #pragma acc parallel copy(data[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            increment_vector(&data[i][0][0], M * P);
        }
    }
}

/* Test conditional offloading with different partitions */
void test_conditional_offloading(int data[N][M][P], int condition) {
    printf("Testing conditional offloading...\n");
    
    #pragma acc parallel if(condition) copy(data[0:N][0:M][0:P]) gang
    {
        if (condition) {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        data[i][j][k] += 1000;
                    }
                }
            }
        }
    }
    
    #pragma acc kernels if(!condition) copy(data[0:N][0:M][0:P]) worker vector
    {
        if (!condition) {
            #pragma acc loop worker vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        data[i][j][k] -= 500;
                    }
                }
            }
        }
    }
}

/* Main function with persistent device data */
int main(int argc, char *argv[]) {
    int (*data)[M][P] = malloc(N * sizeof(*data));
    int (*data2)[M][P] = malloc(N * sizeof(*data2));
    
    if (!data || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                data[i][j][k] = i * 1000 + j * 100 + k;
                data2[i][j][k] = i * 2000 + j * 200 + k;
            }
        }
    }
    
    /* Use argc to create conditional execution paths */
    int test_case = argc > 1 ? atoi(argv[1]) % 9 : 0;
    
    /* Establish persistent device data region with partitioning */
    #pragma acc enter data copyin(data[0:N][0:M][0:P]) gang
    #pragma acc enter data create(data2[0:N][0:M][0:P]) worker
    
    /* Execute different test cases based on input */
    switch (test_case) {
        case 0:
            test_gang_redundant(data);
            break;
        case 1:
            test_gang_partitioned(data);
            break;
        case 2:
            test_worker_partitioned(data);
            break;
        case 3:
            test_gang_worker_partitioned(data);
            break;
        case 4:
            test_vector_partitioned(data);
            break;
        case 5:
            test_gang_vector_partitioned(data);
            break;
        case 6:
            test_worker_vector_partitioned(data);
            break;
        case 7:
            test_fully_partitioned(data);
            break;
        case 8:
            test_nested_regions(data);
            test_conditional_offloading(data2, argc > 2);
            break;
    }
    
    /* Use data2 with different partition in compute region */
    #pragma acc parallel present(data2) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    data2[i][j][k] = data[i][j][k] + data2[i][j][k];
                }
            }
        }
    }
    
    /* Clean up device data */
    #pragma acc exit data copyout(data[0:N][0:M][0:P])
    #pragma acc exit data delete(data2[0:N][0:M][0:P])
    
    /* Verify results on host */
    int errors = 0;
    for (int i = 0; i < N && i < 5; i++) {  /* Check first few elements */
        for (int j = 0; j < M && j < 5; j++) {
            for (int k = 0; k < P && k < 5; k++) {
                int expected = i * 1000 + j * 100 + k;
                /* Apply transformations based on test case */
                switch (test_case) {
                    case 0: expected += i + j + k; break;
                    case 1: expected *= 2; break;
                    case 2: expected -= 5; break;
                    case 3: expected = expected * 3 / 2; break;
                    case 4: expected += 100; break;
                    case 5: expected = expected << 1; break;
                    case 6: expected |= 0xFF; break;
                    case 7: expected = ~expected; break;
                    case 8: 
                        expected = ((expected * 2) + 10) + 1;
                        if (argc > 2) expected += 1000;
                        break;
                }
                if (data[i][j][k] != expected) {
                    errors++;
                    if (errors < 10) {
                        printf("Mismatch at [%d][%d][%d]: got %d, expected %d\n",
                               i, j, k, data[i][j][k], expected);
                    }
                }
            }
        }
    }
    
    if (errors == 0) {
        printf("All tests passed for case %d\n", test_case);
    } else {
        printf("Found %d errors for case %d\n", errors, test_case);
    }
    
    free(data);
    free(data2);
    
    return errors > 0 ? 1 : 0;
}
