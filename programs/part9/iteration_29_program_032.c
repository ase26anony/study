/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int* data, int count) {
    const int* ptr = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment - should generate (reg + 0) */
    for (int i = 0; i < count; i++) {
        /* Critical access: *ptr where ptr hasn't been incremented yet */
        sum += *ptr;    /* This should become (mem (plus (reg) (const_int 0))) */
        ptr++;          /* Post-increment */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int* data, int count, int value) {
    int* ptr = data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        *ptr = value + i;  /* Write pattern - same address form */
        ptr++;             /* Post-increment */
    }
}

int main(int argc, char* argv[]) {
    /* Use argc to make count non-constant for the compiler */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int* array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Process with read loop */
    int result = process_data(array, count);
    
    /* Modify with write loop */
    modify_data(array, count, 10);
    
    /* Use result to prevent elimination */
    printf("Result: %d (array[0] = %d)\n", result, array[0]);
    
    free(array);
    return 0;
}
