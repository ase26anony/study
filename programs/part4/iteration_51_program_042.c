/* test_doloop.c - Test program for GCC doloop optimization coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HOT_LOOP_COUNT 1000
#define ARRAY_SIZE 10000

/* Mark function as hot to encourage loop optimization */
__attribute__((hot))
static void process_data_downward_for(int *arr, int len) {
    /* Pattern 1: Basic decrementing for loop with > condition */
    for (int i = len - 1; i > 0; i--) {
        arr[i] = arr[i] * 3 + 7;
    }
    /* Handle element 0 separately to avoid i >= 0 */
    arr[0] = arr[0] * 3 + 7;
}

__attribute__((hot))
static void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        arr[i - 1] = arr[i - 1] * 2 + 1;
    }
}

__attribute__((hot))
static void process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int n = len;
    while (n--) {
        arr[n] = arr[n] + n;
    }
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len) {
    /* Pattern 4: Nested loop with decrementing inner counter */
    int outer = len / 100;
    for (int j = 0; j < outer; j++) {
        int inner_len = 100;
        /* Inner loop with decrementing counter */
        for (int i = inner_len - 1; i > 0; i--) {
            int idx = j * 100 + i;
            arr[idx] = arr[idx] ^ 0x5A;
        }
        arr[j * 100] = arr[j * 100] ^ 0x5A;
    }
}

__attribute__((hot))
static void process_data_mixed(unsigned *arr, int len) {
    /* Pattern 5: Using unsigned counter (common in doloop) */
    unsigned i = len;
    do {
        i--;
        arr[i] = (arr[i] << 1) | (arr[i] >> 31);
    } while (i > 0);
}

__attribute__((hot))
static long compute_checksum(int *arr, int len) {
    /* Simple checksum to prevent dead code elimination */
    long sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int *data1 = malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = malloc(ARRAY_SIZE * sizeof(int));
    unsigned *data3 = malloc(ARRAY_SIZE * sizeof(unsigned));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
    }
    
    long total_checksum = 0;
    
    /* Execute hot loops many times to encourage optimization */
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        /* Call different loop patterns */
        process_data_downward_for(data1, ARRAY_SIZE);
        process_data_downward_for_ne(data2, ARRAY_SIZE);
        process_data_while_decrement(data1, ARRAY_SIZE);
        process_data_nested_loops(data2, ARRAY_SIZE);
        process_data_mixed(data3, ARRAY_SIZE);
        
        /* Prevent compiler from optimizing everything away */
        if (iter % 100 == 0) {
            total_checksum += compute_checksum(data1, ARRAY_SIZE);
            total_checksum += compute_checksum(data2, ARRAY_SIZE);
        }
    }
    
    /* Final checksum computation */
    total_checksum += compute_checksum(data1, ARRAY_SIZE);
    total_checksum += compute_checksum(data2, ARRAY_SIZE);
    
    printf("Final checksum: %ld\n", total_checksum);
    
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
