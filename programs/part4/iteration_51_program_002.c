/* test_doloop.c
 * Compile with: gcc -O2 -funroll-loops -fdoloop -march=native -c test_doloop.c
 * For ARM: gcc -O2 -fdoloop -march=armv8-a -c test_doloop.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HOT_LOOP_COUNT 1000
#define ARRAY_SIZE 10000

/* Mark function as hot to encourage loop optimizations */
__attribute__((hot))
static void process_data(int *arr, int len) {
    int i;
    int sum = 0;
    
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (i = len - 1; i > 0; i--) {
        arr[i] = arr[i] * 3 + 7;
        sum += arr[i];  /* Prevent dead code elimination */
    }
    
    /* Pattern 2: Decrementing for loop with != condition */
    for (i = len - 1; i != 0; i--) {
        arr[i] = arr[i] + i;
        sum += arr[i];
    }
    
    /* Pattern 3: While loop that decrements counter */
    i = len;
    while (i--) {
        arr[i] = arr[i] - 5;
        sum += arr[i];
    }
    
    /* Pattern 4: Nested loop with decrementing inner counter */
    int j;
    for (j = 0; j < 10; j++) {
        for (i = len - 1; i >= 0; i--) {
            arr[i] = arr[i] ^ j;  /* Simple non-trivial operation */
            sum += arr[i];
        }
    }
    
    /* Use sum to prevent optimization */
    arr[0] = sum % 256;
}

/* Another hot function with different loop patterns */
__attribute__((hot))
static void process_data_unsigned(unsigned *arr, unsigned len) {
    unsigned i;
    
    /* Pattern with unsigned counter */
    for (i = len; i > 0; i--) {
        arr[i-1] = arr[i-1] * 2 + 1;
    }
    
    /* Another variant */
    i = len;
    while (i) {
        i--;
        arr[i] = arr[i] >> 1;
    }
}

int main(int argc, char *argv[]) {
    int *data;
    unsigned *udata;
    int i, j;
    int len = ARRAY_SIZE;
    
    /* Use command line argument for variable loop bound */
    if (argc > 1) {
        len = atoi(argv[1]);
        if (len <= 0) len = ARRAY_SIZE;
    }
    
    /* Allocate and initialize arrays */
    data = (int*)malloc(len * sizeof(int));
    udata = (unsigned*)malloc(len * sizeof(unsigned));
    
    if (!data || !udata) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (i = 0; i < len; i++) {
        data[i] = rand() % 100;
        udata[i] = rand() % 100;
    }
    
    /* Hot loop to make the functions "hot" */
    for (j = 0; j < HOT_LOOP_COUNT; j++) {
        process_data(data, len);
        process_data_unsigned(udata, len);
        
        /* Occasionally modify length to prevent constant propagation */
        if (j % 100 == 0) {
            data[len-1] = j;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < len; i++) {
        checksum += data[i];
        checksum += udata[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Final values: data[0]=%d, udata[0]=%u\n", data[0], udata[0]);
    
    free(data);
    free(udata);
    
    return 0;
}
