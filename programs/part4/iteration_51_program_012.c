/* test_doloop.c
 * Designed to trigger GCC's doloop optimization validation logic
 * Compile with: gcc -O2 -fdoloop -funroll-loops -march=native -c test_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HOT_LOOP_COUNT 1000
#define ARRAY_SIZE 10000

/* Mark function as hot to encourage loop optimizations */
__attribute__((hot))
void process_data(int *arr, int len) {
    int i;
    
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (i = len - 1; i > 0; i--) {
        arr[i] = arr[i] * 2 + 1;
    }
    
    /* Pattern 2: Decrementing for loop with != condition */
    for (i = len - 1; i != 0; i--) {
        arr[i] = arr[i] + arr[i-1];
    }
    
    /* Pattern 3: While loop that decrements counter */
    i = len;
    while (i--) {
        arr[i] = arr[i] ^ 0x55AA55AA;
    }
    
    /* Pattern 4: Nested loop with decrementing inner counter */
    int j;
    for (j = 0; j < 10; j++) {
        for (i = len - 1; i >= 0; i--) {
            arr[i] = arr[i] - j;
        }
    }
}

/* Another hot function with different loop patterns */
__attribute__((hot))
void process_data_unsigned(unsigned *arr, unsigned len) {
    unsigned i;
    
    /* Pattern 5: Unsigned decrementing loop */
    for (i = len; i > 0; i--) {
        arr[i-1] = (arr[i-1] << 1) | (arr[i-1] >> 31);
    }
    
    /* Pattern 6: Countdown with post-decrement */
    i = len;
    while (i) {
        arr[--i] = arr[i] * 3;
    }
}

int main(int argc, char *argv[]) {
    int *data1;
    unsigned *data2;
    int i, j;
    unsigned long checksum = 0;
    
    /* Use runtime value to prevent compile-time unrolling */
    int array_size = argc > 1 ? atoi(argv[1]) : ARRAY_SIZE;
    if (array_size < 100) array_size = 100;
    
    /* Allocate arrays */
    data1 = (int*)malloc(array_size * sizeof(int));
    data2 = (unsigned*)malloc(array_size * sizeof(unsigned));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < array_size; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
    }
    
    /* Execute hot loops many times to encourage doloop optimization */
    for (j = 0; j < HOT_LOOP_COUNT; j++) {
        process_data(data1, array_size);
        process_data_unsigned(data2, array_size);
        
        /* Alternate array sizes slightly to prevent pattern recognition */
        if (j % 2 == 0) {
            process_data(data1, array_size - 1);
        } else {
            process_data(data1, array_size);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < array_size; i++) {
        checksum += data1[i];
        checksum += data2[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    free(data1);
    free(data2);
    
    return 0;
}
