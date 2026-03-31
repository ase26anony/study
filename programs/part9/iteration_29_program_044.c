/* auto_inc_test.c - Test program for auto-increment/decrement recognition */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int* data, int count) {
    const volatile int* p = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        sum += *p;  /* Should generate (reg + 0) address pattern */
        p++;        /* Post-increment */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int* data, int count, int value) {
    volatile int* q = data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        *q = value + i;  /* Should generate (reg + 0) address pattern */
        q++;             /* Post-increment */
    }
}

int main(int argc, char* argv[]) {
    /* Use argc to make count non-constant for optimizer */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    volatile int* array = (volatile int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Call read function - should trigger (reg + 0) pattern */
    int result = process_data(array, count);
    
    /* Call write function - another chance for (reg + 0) pattern */
    modify_data(array, count, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (array[0] = %d)\n", result, array[0]);
    
    free((void*)array);
    return 0;
}
