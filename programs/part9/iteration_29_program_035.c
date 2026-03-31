/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to isolate the loop for RTL analysis */
__attribute__((noinline)) 
static int process_read_loop(volatile int *data, int count) {
    volatile int *ptr = data;
    int sum = 0;
    
    /* Loop with pointer dereference and post-increment
     * Should generate (reg + 0) address pattern */
    for (int i = 0; i < count; i++) {
        sum += *ptr;  /* Dereference pointer (ptr + 0) */
        ptr++;        /* Post-increment */
    }
    
    return sum;
}

/* Second function with write pattern to increase coverage */
__attribute__((noinline))
static void process_write_loop(int *dest, volatile int *src, int count) {
    int *dptr = dest;
    volatile int *sptr = src;
    
    /* Copy loop with post-increment on both sides */
    for (int i = 0; i < count; i++) {
        *dptr = *sptr;  /* Both addresses should be (reg + 0) */
        dptr++;
        sptr++;
    }
}

/* Third variant: mixed read-write with pointer arithmetic */
__attribute__((noinline))
static int process_mixed(volatile int *arr, int count) {
    volatile int *p = arr;
    int result = 0;
    
    while (count-- > 0) {
        int val = *p;      /* Read: (p + 0) */
        result ^= val;     /* Use value */
        p += 1;           /* Increment pointer */
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use command line argument to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    volatile int *array = (volatile int*)malloc(size * sizeof(int));
    int *dest = (int*)malloc(size * sizeof(int));
    
    if (!array || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
        dest[i] = 0;
    }
    
    /* Call functions with different patterns */
    int sum1 = process_read_loop(array, size);
    process_write_loop(dest, array, size);
    int sum2 = process_mixed(array, size);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d (check: %d)\n", 
           sum1, sum2, dest[size/2]);
    
    free((void*)array);
    free(dest);
    
    return 0;
}
