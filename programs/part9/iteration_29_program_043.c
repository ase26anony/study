/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to isolate the loop for analysis */
__attribute__((noinline)) 
static int process_data(const volatile int *data, int count) {
    const int *ptr = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment
     * This should generate (reg + 0) pattern */
    for (int i = 0; i < count; i++) {
        sum += *ptr;  /* Should become (mem (plus (reg) (const_int 0))) */
        ptr++;        /* Post-increment */
    }
    
    return sum;
}

/* Second function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int *data, int count, int value) {
    int *ptr = data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        *ptr = value + i;  /* Another (reg + 0) pattern */
        ptr++;             /* Post-increment */
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make count non-constant for the compiler */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Process with read loop */
    int result = process_data(array, count);
    
    /* Modify with write loop */
    modify_data(array, count, 10);
    
    /* Use results to prevent optimization */
    printf("Sum: %d, First element: %d\n", result, array[0]);
    
    free(array);
    return 0;
}
