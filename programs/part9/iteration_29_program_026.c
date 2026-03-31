/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to isolate the RTL pattern */
__attribute__((noinline)) 
static int process_reads(const volatile int *data, int count) {
    const int *ptr = (const int *)data;
    int sum = 0;
    
    /* Loop with pointer dereference and post-increment */
    for (int i = 0; i < count; i++) {
        /* This should generate (reg + 0) pattern */
        sum += *ptr;
        ptr += 1;  /* Post-increment after use */
    }
    
    return sum;
}

/* Another function with write pattern */
__attribute__((noinline))
static void process_writes(volatile int *data, int count, int value) {
    int *ptr = (int *)data;
    
    /* Loop with pointer dereference and post-increment */
    for (int i = 0; i < count; i++) {
        /* This should also generate (reg + 0) pattern */
        *ptr = value + i;
        ptr += 1;  /* Post-increment after use */
    }
}

int main(int argc, char *argv[]) {
    /* Use command line to prevent compile-time optimization */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    volatile int *array = (volatile int *)malloc(count * sizeof(int));
    if (!array) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < count; i++) {
        array[i] = i * 2;
    }
    
    /* Call the read function - this should trigger the target code */
    int result = process_reads(array, count);
    
    /* Call the write function - increases chance of coverage */
    process_writes(array, count, 42);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Verify with a simple check */
    int verify = 0;
    for (int i = 0; i < count; i++) {
        verify += array[i];
    }
    printf("Verify: %d\n", verify);
    
    free((void *)array);
    return 0;
}
