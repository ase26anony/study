/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
int __attribute__((noinline)) process_data(const volatile int* data, int count) {
    const int* ptr = data;  /* Start pointer */
    int sum = 0;
    
    /* Loop with pointer dereference and post-increment */
    for (int i = 0; i < count; i++) {
        /* This should generate (reg + 0) address pattern */
        sum += *ptr;
        ptr += 1;  /* Post-increment - may become (set reg (plus reg const_int)) */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
void __attribute__((noinline)) write_data(volatile int* data, int count, int value) {
    volatile int* ptr = data;
    
    /* Loop with write and post-increment */
    for (int i = 0; i < count; i++) {
        *ptr = value + i;
        ptr += 1;  /* Post-increment */
    }
}

int main(int argc, char* argv[]) {
    /* Use command line argument to prevent constant propagation */
    int count = 100;
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 100;
    }
    
    /* Allocate and initialize array */
    int* array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Process data with read loop */
    int result = process_data(array, count);
    
    /* Write data with write loop */
    write_data(array, count, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Verify write loop worked */
    printf("First element after write: %d\n", array[0]);
    
    free(array);
    return 0;
}
