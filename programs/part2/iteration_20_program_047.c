/* test_loops.c - Test program for GCC loop-doloop.cc pattern matching */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent optimization from removing loops */
volatile int global_counter = 0;
volatile int sink = 0;

/* External function to prevent inlining */
__attribute__((noinline, noclone))
void use_value(int val) {
    sink = val;
}

/* Function with loops that should generate the target RTL pattern */
__attribute__((noinline, optimize("O1")))
void test_loops(int iterations, int* array) {
    int i, j;
    int local_counter = 0;
    
    /* VARIANT 1: Basic for loop with decrement */
    for (i = iterations; i > 0; i--) {
        /* Non-trivial operation to prevent removal */
        array[i % 256] = i;
        use_value(array[i % 256]);
        local_counter++;
    }
    
    /* VARIANT 2: do-while loop with pre-decrement */
    j = iterations;
    if (j > 0) {
        do {
            array[j % 256] = j * 2;
            use_value(array[j % 256]);
            local_counter--;
        } while (--j > 0);
    }
    
    /* VARIANT 3: while loop with post-decrement */
    int k = iterations;
    while (k--) {
        array[k % 256] = k * 3;
        use_value(array[k % 256]);
        local_counter += 2;
    }
    
    /* VARIANT 4: Nested loops with inner decrementing counter */
    int outer = iterations / 10;
    if (outer < 1) outer = 1;
    
    for (int m = 0; m < outer; m++) {
        int inner = iterations;
        while (inner > 0) {
            array[(m + inner) % 256] = m * inner;
            use_value(array[(m + inner) % 256]);
            inner--;
            local_counter++;
        }
    }
    
    global_counter = local_counter;
}

/* Another function with different optimization level */
#pragma GCC push_options
#pragma GCC optimize("O1")
__attribute__((noinline, noclone))
void more_loops(int n, int* arr) {
    /* Loop that might generate different RTL */
    for (int x = n; x != 0; x = x - 1) {
        arr[x & 0xFF] ^= x;
        use_value(arr[x & 0xFF]);
    }
    
    /* Another variant with explicit comparison */
    int y = n;
    while (y > 0) {
        arr[y & 0xFF] += y;
        use_value(arr[y & 0xFF]);
        y = y - 1;
    }
}
#pragma GCC pop_options

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations < 100) {
        fprintf(stderr, "Iterations should be >= 100 for meaningful test\n");
        iterations = 100;
    }
    
    /* Non-constant array to prevent optimization */
    int* array = (int*)malloc(256 * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 256; i++) {
        array[i] = i + 1;
    }
    
    /* Test the loops */
    test_loops(iterations, array);
    more_loops(iterations / 2, array);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= array[i];
    }
    
    printf("Result: global_counter=%d, checksum=0x%08x\n", 
           global_counter, checksum);
    
    free(array);
    return 0;
}
