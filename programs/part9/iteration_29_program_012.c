/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline))
static int process_data(const volatile int *data, int count) {
    const volatile int *ptr = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* This should generate (reg + 0) address pattern */
        sum += *ptr;
        ptr += 1;  /* Post-increment after access */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int *data, int count, int value) {
    volatile int *ptr = data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* This should also generate (reg + 0) address pattern */
        *ptr = value + i;
        ptr += 1;  /* Post-increment after access */
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time optimization */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    volatile int *array = malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Call the processing function */
    int result = process_data(array, count);
    
    /* Call the modification function */
    modify_data(array, count, 10);
    
    /* Use results to prevent dead code elimination */
    printf("Sum: %d\n", result);
    printf("First element after modification: %d\n", array[0]);
    
    free((void*)array);
    return 0;
}
