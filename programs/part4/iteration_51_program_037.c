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
    
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (i = len - 1; i > 0; i--) {
        arr[i] = arr[i] * 3 + 7;
    }
    arr[0] = arr[0] * 3 + 7;  /* Handle last element separately */
    
    /* Pattern 2: Decrementing for loop with != condition */
    for (i = len - 1; i != 0; i--) {
        arr[i] = arr[i] + i;
    }
    
    /* Pattern 3: While loop that decrements a counter */
    i = len;
    while (i--) {
        arr[i] = arr[i] - (i % 8);
    }
    
    /* Pattern 4: Nested loop with inner decrementing counter */
    int j;
    for (j = 0; j < 10; j++) {
        for (i = len - 1; i >= 0; i--) {
            arr[i] = arr[i] ^ (j * 0x5A5A);
        }
    }
}

/* Another hot function with different loop patterns */
__attribute__((hot))
void process_data2(unsigned int *arr, unsigned int len) {
    unsigned int i;
    
    /* Pattern 5: Unsigned decrementing loop */
    for (i = len; i > 0; i--) {
        arr[i-1] = (arr[i-1] << 1) | (arr[i-1] >> 31);
    }
    
    /* Pattern 6: Countdown to zero with subtraction */
    i = len;
    while (i) {
        arr[i-1] += 0xDEADBEEF;
        i--;
    }
}

int main(int argc, char *argv[]) {
    int *array1;
    unsigned int *array2;
    int array_size = ARRAY_SIZE;
    int i, j;
    unsigned long long checksum = 0;
    
    /* Use command line argument for array size if provided */
    if (argc > 1) {
        array_size = atoi(argv[1]);
        if (array_size <= 0) array_size = ARRAY_SIZE;
    }
    
    /* Allocate arrays */
    array1 = (int*)malloc(array_size * sizeof(int));
    array2 = (unsigned int*)malloc(array_size * sizeof(unsigned int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < array_size; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Execute hot loops many times to encourage optimization */
    for (j = 0; j < HOT_LOOP_COUNT; j++) {
        process_data(array1, array_size);
        process_data2(array2, array_size);
        
        /* Alternate between different array sizes to prevent pattern recognition */
        if (j % 100 == 0) {
            process_data(array1, array_size / 2);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < array_size; i++) {
        checksum += array1[i];
        checksum += array2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Clean up */
    free(array1);
    free(array2);
    
    return 0;
}
