/* test_doloop.c
 * Designed to trigger GCC's doloop optimization validation logic
 * Compile with: gcc -O2 -fdoloop -funroll-loops -c test_doloop.c
 * Or for ARM: gcc -O2 -fdoloop -march=armv8-a -c test_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>

#define ITERATIONS 1000
#define ARRAY_SIZE 1024

/* Mark function as hot to encourage loop optimization */
__attribute__((hot))
static void process_data(int *arr, int len) {
    int i;
    int sum = 0;
    
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (i = len - 1; i > 0; i--) {
        arr[i] = arr[i] * 3 + 7;
        sum += arr[i];
    }
    
    /* Pattern 2: Decrementing for loop with != condition */
    int j = len;
    for (i = 0; i < len; i++) {
        j--;
        if (j != 0) {
            arr[i] += arr[j] * 2;
            sum += arr[i];
        }
    }
    
    /* Pattern 3: While loop that decrements a counter */
    int k = len;
    while (k--) {
        arr[k % len] = (arr[k % len] + sum) & 0xFF;
    }
    
    /* Pattern 4: Nested loop with decrementing inner counter */
    int m, n;
    for (m = 0; m < 10; m++) {
        for (n = len - 1; n >= 0; n--) {
            arr[n] = arr[n] ^ (m * 0x5A5A);
            sum += arr[n];
        }
    }
    
    /* Use sum to prevent dead code elimination */
    arr[0] = sum;
}

/* Another hot function with different loop patterns */
__attribute__((hot))
static void process_data_alt(unsigned *arr, unsigned len) {
    unsigned i;
    
    /* Pattern 5: Unsigned decrementing loop */
    for (i = len; i > 0; i--) {
        arr[i - 1] = (arr[i - 1] * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Pattern 6: Do-while style with decrement */
    unsigned count = len;
    do {
        count--;
        arr[count] += arr[(count * 17) % len];
    } while (count != 0);
}

int main(int argc, char **argv) {
    int *data1;
    unsigned *data2;
    int i, result = 0;
    
    /* Use runtime-determined size to prevent constant folding */
    int size = ARRAY_SIZE;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = ARRAY_SIZE;
    }
    
    /* Allocate arrays */
    data1 = (int*)malloc(size * sizeof(int));
    data2 = (unsigned*)malloc(size * sizeof(unsigned));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-zero values */
    for (i = 0; i < size; i++) {
        data1[i] = i * 3 + 1;
        data2[i] = i * 5 + 2;
    }
    
    /* Repeatedly call hot functions to make loops "hot" */
    for (i = 0; i < ITERATIONS; i++) {
        process_data(data1, size);
        process_data_alt(data2, size);
        
        /* Alternate between different sizes to prevent pattern recognition */
        int alt_size = size;
        if (i % 3 == 0) alt_size = size - 1;
        if (i % 7 == 0) alt_size = size + 1;
        
        process_data(data1, alt_size % size + 1);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < size; i++) {
        result ^= data1[i];
        result += data2[i];
    }
    
    printf("Result checksum: %d\n", result);
    
    free(data1);
    free(data2);
    
    return 0;
}
