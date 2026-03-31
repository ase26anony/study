/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -c auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
int __attribute__((noinline)) process_data(const int* data, int count) {
    const int* ptr = data;
    int sum = 0;
    int i;
    
    /* Loop designed to generate (reg + 0) addressing pattern */
    for (i = 0; i < count; i++) {
        /* Critical access: *ptr where ptr will be (ptr_reg + 0) */
        sum += *ptr;
        /* Post-increment - should be recognized as auto-increment candidate */
        ptr += 1;
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
void __attribute__((noinline)) modify_data(int* data, int count, int value) {
    int* ptr = data;
    int i;
    
    /* Write loop with similar pattern */
    for (i = 0; i < count; i++) {
        *ptr = value + i;
        ptr += 1;
    }
}

/* Third function using volatile to prevent optimization */
int __attribute__((noinline)) process_volatile(volatile int* data, int count) {
    volatile int* ptr = data;
    int sum = 0;
    int i;
    
    /* Using volatile pointer to ensure memory accesses aren't optimized away */
    for (i = 0; i < count; i++) {
        sum += *ptr;
        ptr += 1;
    }
    
    return sum;
}

int main(int argc, char** argv) {
    int array[100];
    int i, result1, result2;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i + 1;
    }
    
    /* Call processing functions - using argc to make count dynamic */
    int count = (argc > 1) ? atoi(argv[1]) : 50;
    if (count > 100) count = 100;
    
    /* First: read-only pattern */
    result1 = process_data(array, count);
    
    /* Second: write pattern */
    modify_data(array, count, 10);
    
    /* Third: volatile pattern */
    result2 = process_volatile(array, count);
    
    /* Print results to prevent elimination */
    printf("Result1: %d, Result2: %d\n", result1, result2);
    
    return 0;
}
