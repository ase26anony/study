/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization */
volatile int g_volatile_sink;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int* data, int count) {
    const int* p = data;
    int sum = 0;
    
    /* Loop with pointer dereference and post-increment
     * Should generate (reg + 0) address pattern */
    for (int i = 0; i < count; i++) {
        /* Critical: dereference pointer before increment
         * This should create memory access with address (p + 0) */
        sum += *p;
        
        /* Post-increment - separate statement to encourage
         * (reg + 0) pattern before auto-inc-dec pass */
        p = p + 1;
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void write_data(int* dest, const int* src, int count) {
    int* d = dest;
    const int* s = src;
    
    /* Copy loop with post-increment on both sides */
    for (int i = 0; i < count; i++) {
        *d = *s;
        d = d + 1;
        s = s + 1;
    }
}

/* Third function with mixed read/write using volatile */
__attribute__((noinline))
static int process_volatile(volatile int* data, int count) {
    volatile int* p = data;
    int sum = 0;
    
    /* Using volatile pointer forces memory accesses to remain */
    for (int i = 0; i < count; i++) {
        sum += *p;
        p = p + 1;  /* Post-increment */
    }
    
    /* Use global volatile sink to prevent dead store elimination */
    g_volatile_sink = sum;
    return sum;
}

int main(int argc, char** argv) {
    /* Use command line argument to prevent constant propagation */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int* data = (int*)malloc(count * sizeof(int));
    int* dest = (int*)malloc(count * sizeof(int));
    
    if (!data || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < count; i++) {
        data[i] = i + 1;
    }
    
    /* Call functions that should trigger auto-inc-dec patterns */
    int sum1 = process_data(data, count);
    write_data(dest, data, count);
    int sum2 = process_volatile(data, count);
    
    /* Use results to prevent elimination */
    printf("Sum1: %d, Sum2: %d, First element: %d\n", 
           sum1, sum2, dest[0]);
    
    free(data);
    free(dest);
    
    return 0;
}
