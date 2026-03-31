/* test_doloop.c - Program to trigger doloop optimization validation logic */
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
        /* Non-trivial computation to prevent loop removal */
        arr[i] = arr[i] * 3 + arr[i - 1];
    }
    /* Handle last element separately to avoid out-of-bounds */
    if (len > 0) {
        arr[0] = arr[0] * 3;
    }
}

__attribute__((hot))
static void process_data_downward_for_ne(int *arr, int len) {
    /* Pattern 2: Decrementing for loop with != condition */
    for (int i = len; i != 0; i--) {
        /* Different computation pattern */
        int idx = i - 1;
        arr[idx] = (arr[idx] << 1) | 0x1;
    }
}

__attribute__((hot))
static int process_data_while_decrement(int *arr, int len) {
    /* Pattern 3: While loop that decrements counter */
    int count = len;
    int sum = 0;
    
    while (count--) {
        /* Accumulate sum to create data dependency */
        sum += arr[count];
        arr[count] ^= 0xAA;  /* Non-trivial operation */
    }
    return sum;
}

__attribute__((hot))
static void process_data_nested_loops(int *arr, int len, int inner_iters) {
    /* Pattern 4: Nested loop with decrementing inner counter */
    for (int outer = 0; outer < len; outer += inner_iters) {
        int limit = (outer + inner_iters < len) ? inner_iters : len - outer;
        
        /* Inner loop with decrementing counter */
        for (int inner = limit - 1; inner >= 0; inner--) {
            int idx = outer + inner;
            arr[idx] = arr[idx] + (idx % 256);
        }
    }
}

__attribute__((hot))
static unsigned long process_data_all_patterns(int *arr, int len) {
    unsigned long checksum = 0;
    
    /* Apply different loop patterns to different array segments */
    int segment = len / 4;
    
    /* Segment 1: downward for with > */
    process_data_downward_for(arr, segment);
    
    /* Segment 2: downward for with != */
    process_data_downward_for_ne(arr + segment, segment);
    
    /* Segment 3: while decrement */
    checksum += process_data_while_decrement(arr + 2 * segment, segment);
    
    /* Segment 4: nested loops */
    process_data_nested_loops(arr + 3 * segment, 
                             len - 3 * segment, 
                             16);  /* inner iterations */
    
    /* Calculate final checksum */
    for (int i = 0; i < len; i++) {
        checksum = (checksum * 31 + arr[i]) & 0xFFFFFFFF;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int *data;
    int data_size = ARRAY_SIZE;
    unsigned long total_checksum = 0;
    
    /* Use command line argument for size if provided */
    if (argc > 1) {
        data_size = atoi(argv[1]);
        if (data_size <= 0) data_size = ARRAY_SIZE;
    }
    
    /* Allocate and initialize array */
    data = (int *)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Hot loop: call processing function many times */
    for (int iteration = 0; iteration < HOT_LOOP_COUNT; iteration++) {
        /* Modify loop count slightly each iteration to prevent 
           compiler from optimizing away the loop */
        int mod_size = data_size - (iteration % 7);
        if (mod_size <= 0) mod_size = data_size;
        
        total_checksum ^= process_data_all_patterns(data, mod_size);
        
        /* Slight modification to data to prevent dead code elimination */
        data[iteration % data_size] = iteration;
    }
    
    printf("Final checksum: %lu\n", total_checksum);
    
    /* Use result to prevent optimization */
    if (total_checksum == 0x12345678) {
        printf("Impossible condition\n");
    }
    
    free(data);
    return 0;
}
