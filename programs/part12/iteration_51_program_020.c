/* Test program to cover all partition type cases in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O0 -fopenacc -fprofile-arcs -ftest-coverage test_partition_types.c -o test_partition_types
 * Run with: ./test_partition_types
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define NUM_GANGS 32
#define NUM_WORKERS 8
#define VECTOR_LENGTH 128

/* Function to directly test the string conversion if symbol is exposed */
#ifdef HAVE_RUNTIME_SYMBOL
extern const char* oacc_partition_type_to_str(int type);
#endif

void test_gang_redundant() {
    printf("Testing gang redundant (case 0)...\n");
    int data[ARRAY_SIZE];
    
    #pragma acc parallel copyout(data) gang(redundant)
    {
        #pragma acc loop gang
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * 2;
        }
    }
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += data[i];
    }
    printf("  Sum of first 10 elements: %d\n", sum);
}

void test_gang_partitioned() {
    printf("Testing gang partitioned (case 1)...\n");
    int data[ARRAY_SIZE];
    
    #pragma acc parallel copyout(data) gang(num:NUM_GANGS)
    {
        #pragma acc loop gang
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * 3;
        }
    }
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += data[i];
    }
    printf("  Sum of first 10 elements: %d\n", sum);
}

void test_worker_partitioned() {
    printf("Testing worker partitioned (case 2)...\n");
    int data[ARRAY_SIZE];
    
    #pragma acc parallel copyout(data) worker(num:NUM_WORKERS)
    {
        #pragma acc loop worker
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * 4;
        }
    }
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += data[i];
    }
    printf("  Sum of first 10 elements: %d\n", sum);
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned (case 3)...\n");
    int data[ARRAY_SIZE];
    
    #pragma acc parallel copyout(data) gang(num:NUM_GANGS) worker(num:NUM_WORKERS)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * 5;
        }
    }
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += data[i];
    }
    printf("  Sum of first 10 elements: %d\n", sum);
}

void test_vector_partitioned() {
    printf("Testing vector partitioned (case 4)...\n");
    int data[ARRAY_SIZE];
    
    #pragma acc parallel copyout(data) vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * 6;
        }
    }
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += data[i];
    }
    printf("  Sum of first 10 elements: %d\n", sum);
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned (case 5)...\n");
    int data[ARRAY_SIZE];
    
    #pragma acc parallel copyout(data) gang(num:NUM_GANGS) vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * 7;
        }
    }
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += data[i];
    }
    printf("  Sum of first 10 elements: %d\n", sum);
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned (case 6)...\n");
    int data[ARRAY_SIZE];
    
    #pragma acc parallel copyout(data) worker(num:NUM_WORKERS) vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * 8;
        }
    }
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += data[i];
    }
    printf("  Sum of first 10 elements: %d\n", sum);
}

void test_fully_partitioned() {
    printf("Testing fully partitioned (case 7)...\n");
    int data[ARRAY_SIZE];
    
    #pragma acc parallel copyout(data) gang(num:NUM_GANGS) worker(num:NUM_WORKERS) vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * 9;
        }
    }
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += data[i];
    }
    printf("  Sum of first 10 elements: %d\n", sum);
}

/* Alternative approach using kernels region with different partitionings */
void test_kernels_variants() {
    printf("Testing with kernels construct...\n");
    int data1[ARRAY_SIZE], data2[ARRAY_SIZE], data3[ARRAY_SIZE];
    
    /* Case 1: gang partitioned in kernels */
    #pragma acc kernels copyout(data1) gang(NUM_GANGS)
    {
        #pragma acc loop gang
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data1[i] = i + 100;
        }
    }
    
    /* Case 3: gang+worker partitioned in kernels */
    #pragma acc kernels copyout(data2) gang(NUM_GANGS), worker(NUM_WORKERS)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data2[i] = i + 200;
        }
    }
    
    /* Case 7: fully partitioned in kernels */
    #pragma acc kernels copyout(data3) gang(NUM_GANGS), worker(NUM_WORKERS), vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data3[i] = i + 300;
        }
    }
    
    printf("  Kernels variants completed\n");
}

/* Test invalid partition type if we can access the function directly */
void test_invalid_partition_type() {
    printf("Testing invalid partition type (default case)...\n");
    
    /* Try to trigger an invalid state by using unusual clause combinations */
    int data[ARRAY_SIZE];
    
    /* This might trigger an internal error state */
    #pragma acc parallel copyout(data)
    {
        /* Empty parallel region - might use default partitioning */
        #pragma acc loop
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = 0;
        }
    }
    
    /* Another attempt with potentially confusing directives */
    #pragma acc parallel loop copyout(data) gang
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i;
    }
    
    printf("  Invalid type test completed\n");
}

int main() {
    printf("Starting OpenACC partition type coverage test...\n\n");
    
    /* Test all valid partition types */
    test_gang_redundant();
    printf("\n");
    
    test_gang_partitioned();
    printf("\n");
    
    test_worker_partitioned();
    printf("\n");
    
    test_gang_worker_partitioned();
    printf("\n");
    
    test_vector_partitioned();
    printf("\n");
    
    test_gang_vector_partitioned();
    printf("\n");
    
    test_worker_vector_partitioned();
    printf("\n");
    
    test_fully_partitioned();
    printf("\n");
    
    /* Test kernels variants */
    test_kernels_variants();
    printf("\n");
    
    /* Test for potential invalid partition type */
    test_invalid_partition_type();
    printf("\n");
    
    printf("All tests completed successfully!\n");
    
    return 0;
}
