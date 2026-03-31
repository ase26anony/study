/* test_doloop.c
 * Designed to trigger GCC's doloop optimization validation logic
 * Compile with: gcc -O2 -fdoloop -funroll-loops -c test_doloop.c
 * Or for ARM: gcc -O2 -fdoloop -march=armv8-a -c test_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>

#define ITERATIONS 1000
#define ARRAY_SIZE 10000

/* Mark function as hot to encourage loop optimizations */
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
        arr[j] = arr[j] / 2 + 1;
        sum += arr[j];
        j--;
    }
    
    /* Pattern 3: While loop that decrements counter */
    int k = len;
    while (k--) {
        arr[k] = arr[k] + k;
        sum += arr[k];
    }
    
    /* Pattern 4: Nested loop with decrementing inner counter */
    int m, n;
    for (m = 0; m < 10; m++) {
        for (n = len - 1; n > 0; n--) {
            arr[n] = arr[n] ^ 0x55;
            sum += arr[n];
        }
    }
    
    /* Use sum to prevent dead code elimination */
    arr[0] = sum % 256;
}

/* Another hot function with different loop patterns */
__attribute__((hot))
void process_data2(unsigned *arr, unsigned len) {
    unsigned i;
    
    /* Pattern 5: Unsigned decrementing loop */
    for (i = len; i > 0; i--) {
        arr[i-1] = (arr[i-1] << 1) | (arr[i-1] >> 31);
    }
    
    /* Pattern 6: Do-while style with decrement */
    unsigned count = len;
    do {
        arr[count-1] += count;
        count--;
    } while (count > 0);
}

int main(int argc, char **argv) {
    int *array1;
    unsigned *array2;
    int i;
    
    /* Use argc to make size runtime-determined (prevents unrolling) */
    int size = ARRAY_SIZE + (argc > 1 ? atoi(argv[1]) % 100 : 0);
    
    /* Allocate arrays */
    array1 = (int*)malloc(size * sizeof(int));
    array2 = (unsigned*)malloc(size * sizeof(unsigned));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < size; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 5 + 2;
    }
    
    /* Repeatedly call hot functions to make loops "hot" */
    for (i = 0; i < ITERATIONS; i++) {
        process_data(array1, size);
        process_data2(array2, size);
        
        /* Alternate between different sizes to avoid pattern recognition */
        if (i % 100 == 0) {
            process_data(array1, size - 10);
            process_data2(array2, size - 10);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum1 = 0;
    unsigned checksum2 = 0;
    for (i = 0; i < size; i++) {
        checksum1 ^= array1[i];
        checksum2 += array2[i];
    }
    
    printf("Checksums: %d %u\n", checksum1, checksum2);
    
    /* Cleanup */
    free(array1);
    free(array2);
    
    return 0;
}
