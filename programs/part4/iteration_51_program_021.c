/* test_doloop.c - Program to trigger doloop optimization validation */
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
    if (len > 0) {
        arr[0] = arr[0] * 3 + 7;
    }
}

__attribute__((hot))
static void process_data_downward_neq(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        arr[i - 1] = arr[i - 1] * 2 + 5;
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
    /* Pattern 4: Nested loops with inner decrementing counter */
    int outer_iters = 10;
    for (int j = 0; j < outer_iters; j++) {
        for (int i = len - 1; i > 0; i--) {
            arr[i] = (arr[i] + j) & 0xFF;
        }
        if (len > 0) {
            arr[0] = (arr[0] + j) & 0xFF;
        }
    }
}

__attribute__((hot))
static void process_data_mixed(unsigned *arr, int len) {
    /* Pattern 5: Using unsigned counter (common in doloop) */
    unsigned count = len;
    while (count > 0) {
        arr[count - 1] = arr[count - 1] ^ 0xAA;
        count--;
    }
}

/* Main processing function containing all patterns */
__attribute__((hot))
static long process_all_patterns(int *arr1, unsigned *arr2, int len) {
    long checksum = 0;
    
    /* Call each pattern multiple times to increase hotness */
    for (int repeat = 0; repeat < 5; repeat++) {
        process_data_downward_for(arr1, len);
        process_data_downward_neq(arr1, len);
        process_data_while_decrement(arr1, len);
        process_data_nested_loops(arr1, len);
        process_data_mixed(arr2, len);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < len; i++) {
        checksum += arr1[i] + arr2[i];
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int len = ARRAY_SIZE;
    
    /* Use command line argument for variable size if provided */
    if (argc > 1) {
        len = atoi(argv[1]);
        if (len <= 0) len = ARRAY_SIZE;
    }
    
    /* Allocate arrays */
    int *arr1 = (int *)malloc(len * sizeof(int));
    unsigned *arr2 = (unsigned *)malloc(len * sizeof(unsigned));
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < len; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
    }
    
    long total_checksum = 0;
    
    /* Hot loop to make the inner loops "hot" for optimization */
    clock_t start = clock();
    for (int iter = 0; iter < HOT_LOOP_COUNT; iter++) {
        total_checksum += process_all_patterns(arr1, arr2, len);
        
        /* Occasionally reinitialize to vary data */
        if (iter % 100 == 0) {
            for (int i = 0; i < len; i++) {
                arr1[i] = (arr1[i] + iter) % 1000;
            }
        }
    }
    clock_t end = clock();
    
    printf("Total checksum: %ld\n", total_checksum);
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return 0;
}
