/* auto_inc_test.c - Test case for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int* data, int count) {
    const int* p = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    /* This should generate (reg + 0) pattern for auto-inc-dec pass */
    for (int i = 0; i < count; i++) {
        sum += *p;  /* Direct pointer dereference - may become (p + 0) */
        p++;        /* Post-increment */
    }
    
    return sum;
}

/* Second non-inlined function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int* data, int count, int value) {
    volatile int* q = data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        *q = value + i;  /* Direct pointer dereference for store */
        q++;             /* Post-increment */
    }
}

int main(int argc, char* argv[]) {
    /* Use command line argument to prevent compile-time optimization */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    volatile int* array = (volatile int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Call the critical function */
    int result = process_data(array, count);
    
    /* Call the write function */
    modify_data(array, count, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Verify the write worked */
    printf("First element after modify: %d\n", array[0]);
    
    free((void*)array);
    return 0;
}
