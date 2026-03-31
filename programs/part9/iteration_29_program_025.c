/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer operations */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int *data, int count) {
    const int *ptr = data;
    int sum = 0;
    
    /* Loop with pointer dereference and post-increment
     * This should generate (reg + 0) address pattern */
    for (int i = 0; i < count; i++) {
        /* Direct pointer dereference - critical for (reg + 0) pattern */
        sum += *ptr;
        
        /* Post-increment - should be recognized as auto-increment candidate */
        ptr = ptr + 1;
    }
    
    return sum + dummy_volatile; /* Add volatile to prevent dead code elimination */
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void write_pattern(int *data, int count, int value) {
    int *ptr = data;
    
    /* Write loop with similar pattern */
    for (int i = 0; i < count; i++) {
        *ptr = value + i;
        ptr = ptr + 1;  /* Post-increment */
    }
}

int main(int argc, char *argv[]) {
    /* Use command line argument to prevent constant propagation */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Call the critical function */
    int result = process_data(array, count);
    
    /* Also test write pattern */
    write_pattern(array, count, 10);
    
    /* Use result to prevent elimination */
    printf("Result: %d\n", result);
    
    /* Verify write pattern worked */
    printf("First element after write: %d\n", array[0]);
    
    free(array);
    return 0;
}
