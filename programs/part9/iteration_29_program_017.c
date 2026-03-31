/* auto_inc_trigger.c - Trigger auto-inc/dec recognition for (reg + 0) pattern */

#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int *data, int count) {
    const volatile int *p = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* This should generate (reg + 0) address pattern */
        sum += *p;
        p += 1;  /* Post-increment - may become (reg + 0) then reg += 4 */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int *data, int count, int value) {
    volatile int *q = data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        *q = value + i;
        q += 1;  /* Post-increment */
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
    
    /* Call the critical functions */
    int result = process_data(array, count);
    modify_data(array, count, 10);
    
    /* Use results to prevent elimination */
    printf("Sum: %d, First element: %d\n", result, array[0]);
    
    free((void*)array);
    return 0;
}
