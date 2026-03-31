/* auto_inc_trigger.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_trigger.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to isolate the RTL pattern */
__attribute__((noinline)) 
int process_data(const volatile int *data, int count) {
    const int *ptr = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment
     * This should generate (reg + 0) pattern for the load address */
    for (int i = 0; i < count; i++) {
        sum += *ptr;    /* Should become: load from (ptr + 0) */
        ptr++;          /* Post-increment */
    }
    
    return sum;
}

/* Second function with write pattern */
__attribute__((noinline))
void write_pattern(volatile int *dest, const volatile int *src, int count) {
    volatile int *d = dest;
    const volatile int *s = src;
    
    /* Loop 2: Copy using pointer post-increment
     * Both load and store should show (reg + 0) patterns */
    for (int i = 0; i < count; i++) {
        *d = *s;    /* Store to (d + 0), load from (s + 0) */
        d++;
        s++;
    }
}

int main(int argc, char *argv[]) {
    /* Use command line to prevent compile-time optimization */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;  /* Simple sequence 1, 2, 3... */
    }
    
    /* Process data - this should trigger the auto-inc-dec pattern */
    int result = process_data(array, count);
    
    /* Second array for write test */
    int *array2 = (int*)malloc(count * sizeof(int));
    if (array2) {
        write_pattern(array2, array, count);
        free(array2);
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", result);
    
    free(array);
    return 0;
}
