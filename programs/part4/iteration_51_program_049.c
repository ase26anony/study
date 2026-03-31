/* test_doloop.c
 * Compile with: gcc -O2 -funroll-loops -fdoloop -march=native -c test_doloop.c
 * For ARM: gcc -O2 -fdoloop -march=armv8-a -c test_doloop.c
 * For coverage: compile with -fprofile-arcs -ftest-coverage, then run
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HOT_LOOP_COUNT 1000
#define ARRAY_SIZE 10000

/* Mark function as hot to encourage loop optimization */
__attribute__((hot))
static void process_data(int *arr, int len) {
    int i;
    
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (i = len - 1; i > 0; i--) {
        arr[i] = arr[i] * 2 + 1;
    }
    /* Handle i=0 case separately to avoid zero-trip loops */
    if (len > 0) {
        arr[0] = arr[0] * 2 + 1;
    }
    
    /* Pattern 2: Decrementing for loop with != condition */
    for (i = len - 1; i != 0; i--) {
        arr[i] += i;
    }
    
    /* Pattern 3: While loop with decrement */
    i = len;
    while (i--) {
        arr[i] ^= 0x55AA;
    }
    
    /* Pattern 4: Nested loops with inner decrementing counter */
    int j;
    for (j = 0; j < 10; j++) {
        for (i = len - 1; i >= 0; i--) {
            arr[i] = (arr[i] << 1) | (arr[i] >> 31);  /* Rotate left by 1 */
        }
    }
}

/* Another hot function with different loop patterns */
__attribute__((hot))
static void process_data2(unsigned *arr, unsigned len) {
    unsigned i = len;
    
    /* Pattern 5: Do-while style with pre-decrement */
    do {
        i--;
        arr[i] = arr[i] * 3;
    } while (i > 0);
    
    /* Pattern 6: Count down from len to 1 (not zero) */
    for (i = len; i >= 1; i--) {
        arr[i-1] += arr[i % len];
    }
}

int main(int argc, char **argv) {
    int *array1;
    unsigned *array2;
    int i, j;
    unsigned long checksum = 0;
    
    /* Use runtime value to prevent compile-time optimization */
    int size = argc > 1 ? atoi(argv[1]) : ARRAY_SIZE;
    
    /* Allocate arrays */
    array1 = (int*)malloc(size * sizeof(int));
    array2 = (unsigned*)malloc(size * sizeof(unsigned));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < size; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Hot loop: call processing functions many times */
    for (j = 0; j < HOT_LOOP_COUNT; j++) {
        process_data(array1, size);
        process_data2(array2, size);
        
        /* Alternate between different sizes to prevent pattern recognition */
        if (j % 2 == 0) {
            process_data(array1, size - 1);
        } else {
            process_data(array1, size - 2);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < size; i++) {
        checksum += array1[i] + array2[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    
    return 0;
}
