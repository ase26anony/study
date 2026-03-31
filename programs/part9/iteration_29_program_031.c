/* auto_inc_test.c - Test program to trigger auto-inc-dec pass */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
int __attribute__((noinline)) process_data(const volatile int* data, int count) {
    const int* p = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment - should generate (reg + 0) pattern */
    for (int i = 0; i < count; i++) {
        sum += *p;  /* This should generate address: (p + 0) */
        p++;        /* Post-increment */
    }
    
    /* Reset pointer for second loop */
    p = data;
    
    /* Loop 2: Another read loop to increase chances */
    int sum2 = 0;
    for (int i = 0; i < count; i++) {
        sum2 += *p;  /* Another (p + 0) pattern */
        p += 1;      /* Alternative increment syntax */
    }
    
    return sum + sum2;
}

/* Another non-inline function with write pattern */
void __attribute__((noinline)) modify_data(volatile int* data, int count, int value) {
    volatile int* q = data;
    
    /* Loop 3: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        *q = value + i;  /* Write with (q + 0) pattern */
        q++;             /* Post-increment */
    }
}

int main(int argc, char* argv[]) {
    /* Use argc to make count non-constant for the optimizer */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    volatile int* array = (volatile int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i * 2;
    }
    
    /* Call the processing function */
    int result = process_data(array, count);
    
    /* Call the modification function */
    modify_data(array, count, 42);
    
    /* Use results to prevent optimization */
    printf("Result: %d\n", result);
    printf("First element after modification: %d\n", array[0]);
    
    free((void*)array);
    return 0;
}
