/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 * Or for x86: gcc -O2 -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to isolate the RTL pattern */
static int __attribute__((noinline)) 
process_data(const volatile int *data, int count) {
    const int *ptr = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment
     * This should generate (reg + 0) pattern for auto-inc-dec pass */
    for (int i = 0; i < count; i++) {
        /* Direct pointer dereference - compiler may generate (ptr + 0) */
        sum += *ptr;
        /* Post-increment to encourage auto-inc-dec transformation */
        ptr = ptr + 1;
    }
    
    return sum;
}

/* Second non-inlined function with write pattern */
static void __attribute__((noinline))
modify_data(volatile int *data, int count, int value) {
    volatile int *ptr = data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* Write through pointer - may generate (ptr + 0) */
        *ptr = value + i;
        /* Post-increment */
        ptr = ptr + 1;
    }
}

int main(int argc, char *argv[]) {
    /* Use command line argument to prevent constant propagation */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Process data - this function contains the critical pattern */
    int result = process_data(array, count);
    
    /* Modify data - additional pattern for auto-inc-dec */
    modify_data(array, count, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Use modified array */
    printf("First element after modification: %d\n", array[0]);
    
    free(array);
    return 0;
}
