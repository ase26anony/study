/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int *data, int count) {
    const volatile int *ptr = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        sum += *ptr;  /* Should generate (reg + 0) address pattern */
        ptr++;        /* Post-increment */
    }
    
    return sum;
}

/* Another non-inline function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int *data, int count, int value) {
    volatile int *ptr = data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        *ptr = value + i;  /* Should generate (reg + 0) address pattern */
        ptr++;             /* Post-increment */
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time optimization */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    volatile int *array = (volatile int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Process data - this should trigger the auto-inc-dec analysis */
    int result = process_data(array, count);
    
    /* Modify data - additional chance to trigger the pattern */
    modify_data(array, count, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Verify modified data */
    int verify = 0;
    for (int i = 0; i < count; i++) {
        verify += array[i];
    }
    printf("Verify: %d\n", verify);
    
    free((void*)array);
    return 0;
}
