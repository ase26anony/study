/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition code string mapping
 * logic in GCC's OpenACC neuter-broadcast pass, specifically covering
 * the switch statement cases for all partition types.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N 32
#define M 16
#define P 8

/* Global arrays to test various partition mappings */
int array_3d[N][M][P];
int array_2d[N][M];
int array_1d[N];

/* Function prototypes */
void test_gang_redundant(void);
void test_gang_partitioned(void);
void test_worker_partitioned(void);
void test_gang_worker_partitioned(void);
void test_vector_partitioned(void);
void test_gang_vector_partitioned(void);
void test_worker_vector_partitioned(void);
void test_fully_partitioned(void);
void test_nested_regions(void);
void test_device_data_env(void);

/* ACC routine with explicit partition */
#pragma acc routine seq
int process_element(int val, int factor);

/* ACC routine with gang partition */
#pragma acc routine gang
int gang_processed(int val, int factor);

/* ACC routine with vector partition */
#pragma acc routine vector
int vector_processed(int val, int factor);

/* Helper function to initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < N; i++) {
        array_1d[i] = i;
        for (int j = 0; j < M; j++) {
            array_2d[i][j] = i * M + j;
            for (int k = 0; k < P; k++) {
                array_3d[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
}

/* Helper function to verify results */
int verify_results(void) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array_1d[i] != i + 1) errors++;
        for (int j = 0; j < M; j++) {
            if (array_2d[i][j] != (i * M + j) * 2) errors++;
            for (int k = 0; k < P; k++) {
                int expected = (i * M * P + j * P + k) * 3;
                if (array_3d[i][j][k] != expected) errors++;
            }
        }
    }
    return errors;
}

/* Test case 0: gang redundant */
void test_gang_redundant(void) {
    #pragma acc parallel copy(array_1d[0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            array_1d[i] = process_element(array_1d[i], 1);
        }
    }
}

/* Test case 1: gang partitioned */
void test_gang_partitioned(void) {
    #pragma acc kernels create(array_2d[0:N][0:M]) gang(static:2)
    {
        #pragma acc loop gang collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                array_2d[i][j] = gang_processed(array_2d[i][j], 2);
            }
        }
    }
}

/* Test case 2: worker partitioned */
void test_worker_partitioned(void) {
    int condition = 1; /* Use runtime condition to prevent dead code elimination */
    
    #pragma acc parallel if(condition) copy(array_1d[0:N]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            array_1d[i] = process_element(array_1d[i], 1);
        }
    }
}

/* Test case 3: gang+worker partitioned */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(array_2d[0:N][0:M]) gang worker
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                array_2d[i][j] = gang_processed(array_2d[i][j], 2);
            }
        }
    }
}

/* Test case 4: vector partitioned */
void test_vector_partitioned(void) {
    #pragma acc kernels copy(array_1d[0:N]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            array_1d[i] = vector_processed(array_1d[i], 1);
        }
    }
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(array_3d[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    array_3d[i][j][k] = vector_processed(array_3d[i][j][k], 3);
                }
            }
        }
    }
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned(void) {
    #pragma acc kernels copy(array_2d[0:N][0:M]) worker vector
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                array_2d[i][j] = process_element(array_2d[i][j], 2);
            }
        }
    }
}

/* Test case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(array_3d[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    array_3d[i][j][k] = gang_processed(array_3d[i][j][k], 3);
                }
            }
        }
    }
}

/* Test nested regions and mixed partition types */
void test_nested_regions(void) {
    /* Outer region with gang partition */
    #pragma acc parallel copy(array_1d[0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            array_1d[i] = gang_processed(array_1d[i], 1);
            
            /* Simulated nested region behavior */
            if (array_1d[i] % 2 == 0) {
                #pragma acc atomic update
                array_1d[i] += 1;
            }
        }
    }
    
    /* Follow with different partition type */
    #pragma acc kernels copy(array_1d[0:N]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            array_1d[i] = vector_processed(array_1d[i], 1);
        }
    }
}

/* Test device data environment with partition clauses */
void test_device_data_env(void) {
    /* Establish device data region with gang partition */
    #pragma acc enter data copyin(array_3d[0:N][0:M][0:P]) gang
    
    /* Multiple compute regions accessing the partitioned data */
    #pragma acc parallel present(array_3d) worker
    {
        #pragma acc loop worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    array_3d[i][j][k] = process_element(array_3d[i][j][k], 3);
                }
            }
        }
    }
    
    #pragma acc parallel present(array_3d) vector
    {
        #pragma acc loop vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    array_3d[i][j][k] = vector_processed(array_3d[i][j][k], 3);
                }
            }
        }
    }
    
    /* Exit data region */
    #pragma acc exit data copyout(array_3d)
}

/* ACC routine implementations */
#pragma acc routine seq
int process_element(int val, int factor) {
    return val * factor;
}

#pragma acc routine gang
int gang_processed(int val, int factor) {
    return val * factor;
}

#pragma acc routine vector
int vector_processed(int val, int factor) {
    return val * factor;
}

int main(int argc, char **argv) {
    int test_selector = 0;
    
    /* Use argc to create conditional execution paths */
    if (argc > 1) {
        test_selector = atoi(argv[1]) % 9; /* 0-8 for all cases */
    }
    
    /* Initialize test data */
    init_arrays();
    
    /* Execute test cases based on selector to prevent dead code elimination */
    switch (test_selector) {
        case 0:
            test_gang_redundant();
            break;
        case 1:
            test_gang_partitioned();
            break;
        case 2:
            test_worker_partitioned();
            break;
        case 3:
            test_gang_worker_partitioned();
            break;
        case 4:
            test_vector_partitioned();
            break;
        case 5:
            test_gang_vector_partitioned();
            break;
        case 6:
            test_worker_vector_partitioned();
            break;
        case 7:
            test_fully_partitioned();
            break;
        case 8:
            test_nested_regions();
            test_device_data_env();
            break;
    }
    
    /* Re-initialize and run all tests when no args (for coverage) */
    if (argc == 1) {
        init_arrays();
        test_gang_redundant();
        
        init_arrays();
        test_gang_partitioned();
        
        init_arrays();
        test_worker_partitioned();
        
        init_arrays();
        test_gang_worker_partitioned();
        
        init_arrays();
        test_vector_partitioned();
        
        init_arrays();
        test_gang_vector_partitioned();
        
        init_arrays();
        test_worker_vector_partitioned();
        
        init_arrays();
        test_fully_partitioned();
        
        init_arrays();
        test_nested_regions();
        
        init_arrays();
        test_device_data_env();
    }
    
    /* Final verification */
    int errors = verify_results();
    if (errors > 0) {
        printf("Found %d errors in array values\n", errors);
        return 1;
    }
    
    printf("All tests completed successfully\n");
    return 0;
}
