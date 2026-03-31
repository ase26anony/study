/* auto_inc_trigger.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -c auto_inc_trigger.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int *data, int count) {
    const int *ptr = data;
    int sum = 0;
    int i;
    
    /* Loop 1: Read using pointer post-increment
     * This should generate (reg + 0) pattern */
    for (i = 0; i < count; i++) {
        /* Direct pointer dereference followed by increment
         * Should produce: sum += *(ptr + 0); ptr = ptr + 1; */
        sum += *ptr;
        ptr += 1;  /* Post-increment style */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void modify_data(int *data, int count, int value) {
    int *ptr = data;
    int i;
    
    /* Loop 2: Write using pointer post-increment */
    for (i = 0; i < count; i++) {
        /* Write pattern: *(ptr + 0) = value; ptr = ptr + 1; */
        *ptr = value + i;
        ptr += 1;
    }
}

/* Use volatile to prevent optimization of pointer arithmetic */
__attribute__((noinline))
static int process_volatile(volatile int *data, int count) {
    volatile int *ptr = data;
    int sum = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        /* Volatile access forces memory operation preservation */
        sum += *ptr;
        ptr += 1;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int size = 100;
    int *array;
    int result1, result2;
    
    /* Get size from command line to obscure compile-time knowledge */
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate and initialize array */
    array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Call processing functions - compiler can't see inside */
    result1 = process_data(array, size);
    modify_data(array, size, 10);
    result2 = process_volatile(array, size);
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %d, Result2: %d\n", result1, result2);
    
    free(array);
    return 0;
}
