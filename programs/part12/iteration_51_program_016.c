/* Test program to cover omp-oacc-neuter-broadcast.cc partition type string conversion */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ARRAY_SIZE 1000

/* Helper function to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Direct test of the conversion function if it's exposed */
#ifdef TEST_DIRECT
/* Assuming the function signature is something like: */
extern "C" const char* oacc_partition_type_to_str(int type);

void test_direct_conversion() {
    printf("Direct conversion tests:\n");
    for (int i = -1; i <= 8; i++) {
        const char *str = oacc_partition_type_to_str(i);
        printf("  Type %d: %s\n", i, str);
    }
}
#endif

int main() {
    int *data = (int*)malloc(N * sizeof(int));
    int *result = (int*)malloc(N * sizeof(int));
    
    if (!data || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        data[i] = i;
        result[i] = 0;
    }
    
    /* Case 0: gang redundant */
    printf("Testing gang redundant (case 0)...\n");
    #pragma acc parallel gang(redundant) copy(data[0:N], result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = data[i] * 2;
        }
    }
    use(result);
    
    /* Case 1: gang partitioned */
    printf("Testing gang partitioned (case 1)...\n");
    #pragma acc parallel gang(num:32) copy(data[0:N], result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = data[i] + 1;
        }
    }
    use(result);
    
    /* Case 2: worker partitioned */
    printf("Testing worker partitioned (case 2)...\n");
    #pragma acc parallel worker(num:4) copy(data[0:N], result[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            result[i] = data[i] * 3;
        }
    }
    use(result);
    
    /* Case 3: gang+worker partitioned */
    printf("Testing gang+worker partitioned (case 3)...\n");
    #pragma acc parallel gang(num:16) worker(num:8) copy(data[0:N], result[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            result[i] = data[i] - 1;
        }
    }
    use(result);
    
    /* Case 4: vector partitioned */
    printf("Testing vector partitioned (case 4)...\n");
    #pragma acc parallel vector_length(128) copy(data[0:N], result[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = data[i] / 2;
        }
    }
    use(result);
    
    /* Case 5: gang+vector partitioned */
    printf("Testing gang+vector partitioned (case 5)...\n");
    #pragma acc parallel gang(num:8) vector_length(64) copy(data[0:N], result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = data[i] % 100;
        }
    }
    use(result);
    
    /* Case 6: worker+vector partitioned */
    printf("Testing worker+vector partitioned (case 6)...\n");
    #pragma acc parallel worker(num:4) vector_length(32) copy(data[0:N], result[0:N])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            result[i] = data[i] | 0xFF;
        }
    }
    use(result);
    
    /* Case 7: fully partitioned */
    printf("Testing fully partitioned (case 7)...\n");
    #pragma acc parallel gang(num:4) worker(num:2) vector_length(16) copy(data[0:N], result[0:N])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            result[i] = data[i] & 0x0F;
        }
    }
    use(result);
    
    /* Additional tests using kernels construct */
    printf("Testing with kernels construct...\n");
    
    /* Test gang redundant with kernels */
    #pragma acc kernels gang(redundant) copy(data[0:N], result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = data[i] * 4;
        }
    }
    use(result);
    
    /* Test gang partitioned with kernels */
    #pragma acc kernels gang(num:8) copy(data[0:N], result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = data[i] + 10;
        }
    }
    use(result);
    
    /* Test mixed partitioning with kernels */
    #pragma acc kernels gang(num:4) worker(num:2) vector_length(32) copy(data[0:N], result[0:N])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            result[i] = data[i] ^ 0xAA;
        }
    }
    use(result);
    
    /* Test to potentially trigger invalid partition type */
    printf("Testing edge cases...\n");
    
    /* Try nested parallelism which might create unusual partition types */
    #pragma acc parallel copy(data[0:10], result[0:10])
    {
        #pragma acc loop
        for (int i = 0; i < 10; i++) {
            #pragma acc atomic
            result[i] += data[i];
        }
    }
    use(result);
    
    /* Test with async clause which might affect partitioning */
    #pragma acc parallel gang(num:2) async(1) copy(data[0:10], result[0:10])
    {
        #pragma acc loop gang
        for (int i = 0; i < 10; i++) {
            result[i] = data[i] * 5;
        }
    }
    #pragma acc wait(1)
    
    /* Test with if clause */
    int use_acc = 1;
    #pragma acc parallel gang(num:2) if(use_acc) copy(data[0:10], result[0:10])
    {
        #pragma acc loop gang
        for (int i = 0; i < 10; i++) {
            result[i] = data[i] + 100;
        }
    }
    
    /* Test with reduction */
    int sum = 0;
    #pragma acc parallel gang(redundant) reduction(+:sum) copy(data[0:N])
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    use(&sum);
    
    /* Test with private variables */
    int private_var = 42;
    #pragma acc parallel gang(num:4) private(private_var) copy(data[0:10], result[0:10])
    {
        private_var = 100;
        #pragma acc loop gang
        for (int i = 0; i < 10; i++) {
            result[i] = data[i] + private_var;
        }
    }
    
    /* Test with collapse */
    int matrix[10][10];
    #pragma acc parallel gang(num:2) copy(matrix)
    {
        #pragma acc loop gang collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                matrix[i][j] = i * 10 + j;
            }
        }
    }
    use(matrix);
    
    /* Test with tile clause if supported */
    #ifdef ACC_TILE_SUPPORT
    #pragma acc parallel gang(num:2) copy(matrix)
    {
        #pragma acc loop gang tile(2,2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                matrix[i][j] = matrix[i][j] * 2;
            }
        }
    }
    #endif
    
    free(data);
    free(result);
    
    printf("All tests completed.\n");
    
    #ifdef TEST_DIRECT
    test_direct_conversion();
    #endif
    
    return 0;
}
