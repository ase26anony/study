/* test_doloop.c
 * Designed to trigger GCC's doloop optimization validation logic
 * Compile with: gcc -O2 -fdoloop -funroll-loops -march=native -c test_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ITERATIONS 1000
#define ARRAY_SIZE 10000

/* Hot function attribute to encourage optimization */
__attribute__((hot))
void process_data(int *arr, int len) {
    int i;
    
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (i = len - 1; i > 0; i--) {
        arr[i] = arr[i] * 3 + 7;
    }
    
    /* Pattern 2: Decrementing for loop with != condition */
    for (i = len - 1; i != 0; i--) {
        arr[i] = arr[i] + arr[i-1];
    }
    
    /* Pattern 3: While loop with decrement */
    i = len;
    while (i--) {
        arr[i] = arr[i] >> 1;
    }
    
    /* Pattern 4: Nested loop with inner decrementing counter */
    int j;
    for (j = 0; j < 10; j++) {
        for (i = len - 1; i >= 0; i--) {
            arr[i] = arr[i] ^ 0x55;
        }
    }
}

/* Another hot function with different loop patterns */
__attribute__((hot))
void process_data_unsigned(unsigned *arr, unsigned len) {
    unsigned i;
    
    /* Pattern 5: Unsigned decrementing loop */
    for (i = len; i > 0; i--) {
        arr[i-1] = arr[i-1] * 2 + 1;
    }
    
    /* Pattern 6: Do-while with decrement */
    i = len;
    if (i > 0) {
        do {
            arr[--i] = arr[i] | 0xFF;
        } while (i > 0);
    }
}

int main(int argc, char *argv[]) {
    int *data1;
    unsigned *data2;
    int len = ARRAY_SIZE;
    int i, j;
    unsigned long checksum = 0;
    
    /* Use command line argument for variable loop bound */
    if (argc > 1) {
        len = atoi(argv[1]);
        if (len <= 0 || len > 1000000) {
            len = ARRAY_SIZE;
        }
    }
    
    /* Allocate arrays */
    data1 = (int*)malloc(len * sizeof(int));
    data2 = (unsigned*)malloc(len * sizeof(unsigned));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < len; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
    }
    
    /* Repeatedly call hot functions to make loops "hot" */
    for (j = 0; j < ITERATIONS; j++) {
        process_data(data1, len);
        process_data_unsigned(data2, len);
        
        /* Alternate between different lengths occasionally */
        if (j % 100 == 0) {
            process_data(data1, len / 2);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < len; i++) {
        checksum += data1[i] + data2[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    /* Clean up */
    free(data1);
    free(data2);
    
    return 0;
}
