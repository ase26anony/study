/* test_doloop.c - Program to trigger doloop optimization validation logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HOT_LOOP_COUNT 1000
#define ARRAY_SIZE 10000

/* Mark function as hot to encourage loop optimization */
__attribute__((hot))
void process_data(int *arr, int len) {
    int i;
    int sum = 0;
    
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (i = len - 1; i > 0; i--) {
        arr[i] = arr[i] * 3 + 7;
        sum += arr[i];
    }
    
    /* Pattern 2: Decrementing for loop with != condition */
    int j = len - 1;
    while (j != 0) {
        arr[j] = arr[j] + sum;
        j--;
    }
    
    /* Pattern 3: While loop that decrements counter */
    int k = len;
    while (k--) {
        arr[k] = arr[k] * 2 - 1;
    }
    
    /* Pattern 4: Nested loop with decrementing inner counter */
    int m, n;
    for (m = 0; m < 10; m++) {
        for (n = len - 1; n >= 0; n--) {
            arr[n] = arr[n] + m;
        }
    }
    
    /* Prevent dead code elimination */
    arr[0] = sum;
}

/* Another hot function with different loop patterns */
__attribute__((hot))
void process_data2(unsigned *arr, unsigned len) {
    unsigned i;
    
    /* Pattern 5: Unsigned decrementing loop */
    for (i = len; i > 0; i--) {
        arr[i-1] = (arr[i-1] << 1) | (arr[i-1] >> 31);
    }
    
    /* Pattern 6: Do-while with decrement */
    unsigned count = len;
    do {
        arr[count-1] += count;
        count--;
    } while (count > 0);
}

int main(int argc, char *argv[]) {
    int *array1;
    unsigned *array2;
    int i, iterations;
    clock_t start, end;
    double cpu_time_used;
    
    /* Use command line argument or default for iteration count */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = HOT_LOOP_COUNT;
    } else {
        iterations = HOT_LOOP_COUNT;
    }
    
    /* Allocate arrays */
    array1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    array2 = (unsigned*)malloc(ARRAY_SIZE * sizeof(unsigned));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    printf("Starting doloop test with %d iterations...\n", iterations);
    start = clock();
    
    /* Repeatedly call hot functions to make loops "hot" */
    for (i = 0; i < iterations; i++) {
        process_data(array1, ARRAY_SIZE);
        process_data2(array2, ARRAY_SIZE);
        
        /* Alternate between different array sizes to avoid pattern recognition */
        if (i % 2 == 0) {
            process_data(array1, ARRAY_SIZE / 2);
        } else {
            process_data2(array2, ARRAY_SIZE / 4);
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum1 = 0, checksum2 = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum1 += array1[i];
        checksum2 += array2[i];
    }
    
    printf("Checksum1: %lld, Checksum2: %lld\n", checksum1, checksum2);
    printf("Time elapsed: %.2f seconds\n", cpu_time_used);
    
    free(array1);
    free(array2);
    
    return 0;
}
