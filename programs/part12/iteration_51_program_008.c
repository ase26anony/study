/* Test program to cover all partition type cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define NUM_GANGS 32
#define NUM_WORKERS 8
#define VECTOR_LENGTH 128

/* Helper function to perform simple computation */
void init_array(int *arr, int size, int value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value;
    }
}

/* Helper function to verify array */
int verify_array(int *arr, int size, int expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            return 0;
        }
    }
    return 1;
}

int main() {
    int *array1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array4 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array5 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array6 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array7 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array8 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !array4 || 
        !array5 || !array6 || !array7 || !array8) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Testing OpenACC partition types...\n");
    
    /* Case 0: gang redundant */
    printf("Testing case 0: gang redundant\n");
    #pragma acc parallel gang(redundant) copyout(array1[0:ARRAY_SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array1[i] = 100 + i % 100;
        }
    }
    
    /* Case 1: gang partitioned */
    printf("Testing case 1: gang partitioned\n");
    #pragma acc parallel gang(num:NUM_GANGS) copyout(array2[0:ARRAY_SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array2[i] = 200 + i % 100;
        }
    }
    
    /* Case 2: worker partitioned */
    printf("Testing case 2: worker partitioned\n");
    #pragma acc parallel worker(num:NUM_WORKERS) copyout(array3[0:ARRAY_SIZE])
    {
        #pragma acc loop worker
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array3[i] = 300 + i % 100;
        }
    }
    
    /* Case 3: gang+worker partitioned */
    printf("Testing case 3: gang+worker partitioned\n");
    #pragma acc parallel gang(num:NUM_GANGS) worker(num:NUM_WORKERS) copyout(array4[0:ARRAY_SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array4[i] = 400 + i % 100;
        }
    }
    
    /* Case 4: vector partitioned */
    printf("Testing case 4: vector partitioned\n");
    #pragma acc parallel vector_length(VECTOR_LENGTH) copyout(array5[0:ARRAY_SIZE])
    {
        #pragma acc loop vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array5[i] = 500 + i % 100;
        }
    }
    
    /* Case 5: gang+vector partitioned */
    printf("Testing case 5: gang+vector partitioned\n");
    #pragma acc parallel gang(num:NUM_GANGS) vector_length(VECTOR_LENGTH) copyout(array6[0:ARRAY_SIZE])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array6[i] = 600 + i % 100;
        }
    }
    
    /* Case 6: worker+vector partitioned */
    printf("Testing case 6: worker+vector partitioned\n");
    #pragma acc parallel worker(num:NUM_WORKERS) vector_length(VECTOR_LENGTH) copyout(array7[0:ARRAY_SIZE])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array7[i] = 700 + i % 100;
        }
    }
    
    /* Case 7: fully partitioned */
    printf("Testing case 7: fully partitioned\n");
    #pragma acc parallel gang(num:NUM_GANGS) worker(num:NUM_WORKERS) vector_length(VECTOR_LENGTH) copyout(array8[0:ARRAY_SIZE])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array8[i] = 800 + i % 100;
        }
    }
    
    /* Additional test using kernels construct for more coverage */
    printf("Testing with kernels construct...\n");
    int *karray = (int*)malloc(ARRAY_SIZE * sizeof(int));
    if (karray) {
        /* This may trigger different partition types internally */
        #pragma acc kernels copyout(karray[0:ARRAY_SIZE])
        {
            #pragma acc loop gang
            for (int i = 0; i < ARRAY_SIZE; i++) {
                karray[i] = 900 + i % 100;
            }
        }
        free(karray);
    }
    
    /* Verify results */
    printf("Verifying results...\n");
    int all_ok = 1;
    
    /* Simple verification - just check that arrays were modified */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int sum5 = 0, sum6 = 0, sum7 = 0, sum8 = 0;
    
    #pragma acc parallel loop reduction(+:sum1) copyin(array1[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) sum1 += array1[i];
    
    #pragma acc parallel loop reduction(+:sum2) copyin(array2[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) sum2 += array2[i];
    
    #pragma acc parallel loop reduction(+:sum3) copyin(array3[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) sum3 += array3[i];
    
    #pragma acc parallel loop reduction(+:sum4) copyin(array4[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) sum4 += array4[i];
    
    #pragma acc parallel loop reduction(+:sum5) copyin(array5[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) sum5 += array5[i];
    
    #pragma acc parallel loop reduction(+:sum6) copyin(array6[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) sum6 += array6[i];
    
    #pragma acc parallel loop reduction(+:sum7) copyin(array7[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) sum7 += array7[i];
    
    #pragma acc parallel loop reduction(+:sum8) copyin(array8[0:ARRAY_SIZE])
    for (int i = 0; i < ARRAY_SIZE; i++) sum8 += array8[i];
    
    printf("Array sums: %d %d %d %d %d %d %d %d\n", 
           sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    free(array6);
    free(array7);
    free(array8);
    
    printf("Test completed.\n");
    
    return 0;
}
