/* auto_inc_test.c - Test case for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int* data, int count) {
    const int* p = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        sum += *p;  /* Should generate (reg + 0) address */
        p++;        /* Post-increment */
    }
    
    /* Reset pointer for second loop */
    p = data;
    
    /* Loop 2: Another read pattern to increase chances */
    for (int i = 0; i < count; i++) {
        int val = *p;  /* Another (reg + 0) pattern */
        sum += val;
        p = p + 1;     /* Alternative increment syntax */
    }
    
    return sum;
}

/* Another non-inline function with write pattern */
__attribute__((noinline))
static void write_pattern(volatile int* data, int count, int value) {
    volatile int* p = data;
    
    /* Loop 3: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        *p = value + i;  /* Write with (reg + 0) address */
        p++;             /* Post-increment */
    }
}

int main(int argc, char** argv) {
    /* Use command line argument to prevent constant propagation */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int* array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Process with read pattern */
    int result = process_data(array, count);
    
    /* Process with write pattern */
    write_pattern(array, count, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (array[0] = %d)\n", result, array[0]);
    
    free(array);
    return 0;
}
