/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass recognition of (reg + 0) pattern
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer arithmetic */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int *data, int count) {
    const int *ptr = data;
    int sum = 0;
    int i;
    
    /* Loop 1: Read using pointer post-increment
     * This should generate (reg + 0) pattern for the load address */
    for (i = 0; i < count; i++) {
        /* Direct pointer dereference - compiler may generate (ptr + 0) */
        sum += *ptr;
        /* Post-increment - crucial for auto-inc-dec recognition */
        ptr += 1;
    }
    
    /* Use volatile to prevent dead code elimination */
    dummy_volatile = sum;
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void write_data(int *data, int count, int value) {
    int *ptr = data;
    int i;
    
    /* Loop 2: Write using pointer post-increment */
    for (i = 0; i < count; i++) {
        /* Direct pointer dereference for store */
        *ptr = value + i;
        /* Post-increment */
        ptr += 1;
    }
    
    /* Force side effect */
    dummy_volatile = (int)ptr;
}

int main(int argc, char *argv[]) {
    /* Use command line argument to make count non-constant */
    int count = 10;
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 10;
    }
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i * 2;
    }
    
    /* Call processing functions */
    write_data(array, count, 5);
    int result = process_data(array, count);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    dummy_volatile = result;
    
    free(array);
    return 0;
}
