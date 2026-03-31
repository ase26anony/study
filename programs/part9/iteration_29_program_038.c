/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -c auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
int __attribute__((noinline)) process_data(const int *data, int count) {
    const int *ptr = data;
    int sum = 0;
    int i;
    
    /* Critical loop: pointer dereference with post-increment
     * Should generate (reg + 0) address pattern */
    for (i = 0; i < count; i++) {
        /* Direct pointer dereference - compiler may see this as (ptr + 0) */
        sum += *ptr;
        ptr += 1;  /* Post-increment */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
void __attribute__((noinline)) modify_data(int *data, int count, int value) {
    int *ptr = data;
    int i;
    
    /* Write loop with similar pattern */
    for (i = 0; i < count; i++) {
        *ptr = value + i;
        ptr += 1;  /* Post-increment */
    }
}

/* Third function with volatile to prevent optimization */
int __attribute__((noinline)) process_volatile(volatile int *data, int count) {
    volatile int *ptr = data;
    int sum = 0;
    int i;
    
    /* Using volatile pointer to ensure memory access isn't optimized away */
    for (i = 0; i < count; i++) {
        sum += *ptr;
        ptr += 1;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int array[100];
    int i, result;
    
    /* Initialize array with values */
    for (i = 0; i < 100; i++) {
        array[i] = i + 1;
    }
    
    /* Call the critical function - using argc to obscure loop count */
    int count = (argc > 1) ? atoi(argv[1]) : 50;
    if (count > 100) count = 100;
    
    /* Test read pattern */
    result = process_data(array, count);
    printf("Sum after read: %d\n", result);
    
    /* Test write pattern */
    modify_data(array, count, 10);
    
    /* Test volatile pattern */
    result = process_volatile(array, count);
    printf("Sum after volatile read: %d\n", result);
    
    return 0;
}
