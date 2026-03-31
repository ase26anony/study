/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int *data, int count) {
    const volatile int *p = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* This should generate (reg + 0) pattern for the address */
        sum += *p;
        p += 1;  /* Post-increment - may become (post_inc reg) */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int *data, int count, int value) {
    volatile int *q = data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* Another opportunity for (reg + 0) pattern */
        *q = value + i;
        q += 1;  /* Post-increment */
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time optimization */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    volatile int *array = (volatile int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Process data - this contains the critical pattern */
    int result = process_data(array, size);
    
    /* Modify data - additional pattern */
    modify_data(array, size, 10);
    
    /* Use results to prevent elimination */
    printf("Result: %d, First element: %d\n", result, array[0]);
    
    free((void*)array);
    return 0;
}
