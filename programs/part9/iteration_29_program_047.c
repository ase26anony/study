/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int *data, int count) {
    const int *ptr = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* This should generate (reg + 0) pattern */
        sum += *ptr;
        ptr += 1;  /* Post-increment after use */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int *data, int count, int value) {
    int *ptr = (int *)data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        *ptr = value + i;
        ptr += 1;  /* Post-increment after use */
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time optimization */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    volatile int *array = (volatile int *)malloc(count * sizeof(int));
    if (!array) return 1;
    
    /* Initialize with non-zero values */
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Call the critical functions */
    int result = process_data(array, count);
    modify_data(array, count, 10);
    
    /* Use results to prevent elimination */
    printf("Sum: %d, First element: %d\n", result, array[0]);
    
    free((void *)array);
    return 0;
}
