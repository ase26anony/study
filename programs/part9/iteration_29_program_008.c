/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Force no inlining to isolate the critical loop */
__attribute__((noinline))
static int process_read_loop(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;
    
    /* Loop with pointer dereference and post-increment
     * This should generate (reg + 0) address pattern */
    for (int i = 0; i < count; i++) {
        /* Critical: Direct pointer dereference before increment
         * Should create memory access with address (p + 0) */
        sum += *p;
        p += 1;  /* Post-increment - separate statement */
    }
    
    return sum;
}

/* Second function with write pattern */
__attribute__((noinline))
static void process_write_loop(int *ptr, int count, int value) {
    int *p = ptr;
    
    /* Similar pattern but for writes */
    for (int i = 0; i < count; i++) {
        *p = value + i;
        p += 1;  /* Post-increment */
    }
}

/* Use volatile to prevent optimization of pointer arithmetic */
__attribute__((noinline))
static int process_volatile_loop(volatile int *ptr, int count) {
    int sum = 0;
    volatile int *p = ptr;
    
    /* Volatile pointer forces memory access preservation */
    for (int i = 0; i < count; i++) {
        sum += *p;
        p += 1;  /* Post-increment preserved due to volatile */
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command line argument to make count non-constant
     * This prevents loop unrolling and constant propagation */
    int count = 100;
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 100;
    }
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Call the critical functions */
    int sum1 = process_read_loop(array, count);
    
    process_write_loop(array, count, 10);
    
    /* Use volatile pointer version */
    int sum2 = process_volatile_loop(array, count);
    
    /* Print results to prevent dead code elimination */
    printf("Sum1: %d, Sum2: %d\n", sum1, sum2);
    
    /* Use result in conditional to prevent optimization */
    if (sum1 != sum2) {
        printf("Unexpected difference\n");
    }
    
    free(array);
    return 0;
}
