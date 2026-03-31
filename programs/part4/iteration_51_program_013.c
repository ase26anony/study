/* test_doloop.c - Program to trigger GCC's doloop optimization validation */

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
        arr[i] = arr[i] * 2 + 1;
    }
    /* Handle element 0 separately to avoid i >= 0 */
    arr[0] = arr[0] * 2 + 1;
    
    /* Pattern 2: Decrementing for loop with != condition */
    int j = len - 1;
    for (; j != 0; j--) {
        arr[j] += (arr[j] % 7);
    }
    arr[0] += (arr[0] % 7);
    
    /* Pattern 3: While loop that decrements a counter */
    int k = len;
    while (k--) {
        arr[k] ^= 0x55;
    }
    
    /* Pattern 4: Nested loop with decrementing inner counter */
    int m, n;
    for (m = 0; m < 10; m++) {
        for (n = len - 1; n > 0; n--) {
            arr[n] += m;
        }
        arr[0] += m;
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
    
    /* Pattern 6: Do-while with decrement */
    unsigned j = len;
    if (j > 0) {
        do {
            j--;
            arr[j] += j;
        } while (j > 0);
    }
}

int main(int argc, char *argv[]) {
    int *data1;
    unsigned *data2;
    int i, len;
    unsigned long checksum = 0;
    
    /* Use runtime-determined size to prevent compile-time unrolling */
    len = (argc > 1) ? atoi(argv[1]) : ARRAY_SIZE;
    if (len <= 0) len = ARRAY_SIZE;
    
    /* Allocate arrays */
    data1 = (int*)malloc(len * sizeof(int));
    data2 = (unsigned*)malloc(len * sizeof(unsigned));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < len; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
    }
    
    /* Hot loop calling the target functions many times */
    for (i = 0; i < HOT_LOOP_COUNT; i++) {
        process_data(data1, len);
        process_data_unsigned(data2, len);
        
        /* Alternate between different lengths to avoid pattern recognition */
        if (i % 2 == 0) {
            process_data(data1, len - (i % 10));
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < len; i++) {
        checksum += data1[i] + data2[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    free(data1);
    free(data2);
    
    return 0;
}
