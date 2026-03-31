/* auto_inc_test.c - Test case for auto-inc-dec pass */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer arithmetic */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int *data, int count) {
    const int *p = data;
    int sum = 0;
    int i;
    
    /* Loop 1: Read using pointer post-increment */
    for (i = 0; i < count; i++) {
        /* Critical pattern: *p where p will be (reg + 0) */
        sum += *p;
        /* Post-increment - should become auto-increment */
        p++;
    }
    
    /* Use volatile to prevent dead code elimination */
    dummy_volatile = sum;
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void write_data(int *data, int count, int value) {
    int *p = data;
    int i;
    
    /* Loop 2: Write using pointer post-increment */
    for (i = 0; i < count; i++) {
        /* Critical pattern: *p = value where p is (reg + 0) */
        *p = value + i;
        /* Post-increment */
        p++;
    }
}

int main(int argc, char *argv[]) {
    int count = 100;
    int *array;
    int result;
    
    /* Use command line argument to make count less predictable */
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 100;
    }
    
    /* Allocate and initialize array */
    array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Call the critical functions */
    write_data(array, count, 10);
    result = process_data(array, count);
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", result);
    
    /* Use array to prevent elimination */
    printf("First element: %d\n", array[0]);
    
    free(array);
    return 0;
}
