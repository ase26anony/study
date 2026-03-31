/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int* data, int count) {
    const int* p = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* Direct pointer dereference - should generate (reg + 0) pattern */
        sum += *p;
        p += 1;  /* Post-increment - separate statement to encourage (reg+0) */
    }
    
    /* Loop 2: Another read loop to increase chances */
    p = data;
    int sum2 = 0;
    for (int i = 0; i < count; i++) {
        /* Alternative: use in expression then increment */
        int val = *p;
        sum2 ^= val;  /* Different operation to avoid elimination */
        p = p + 1;    /* Explicit addition */
    }
    
    return sum + sum2;
}

/* Another non-inline function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int* data, int count, int value) {
    int* p = (int*)data;
    
    /* Write loop with post-increment */
    for (int i = 0; i < count; i++) {
        *p = value + i;
        p++;  /* Post-increment operator */
    }
}

int main(int argc, char** argv) {
    /* Use argc to make count dynamic and prevent optimization */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    volatile int* array = (volatile int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Call processing functions */
    int result = process_data(array, count);
    modify_data(array, count, 42);
    
    /* Use results to prevent elimination */
    printf("Result: %d\n", result);
    printf("First element after modify: %d\n", array[0]);
    
    free((void*)array);
    return 0;
}
