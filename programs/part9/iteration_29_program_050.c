/* auto_inc_test.c - Test program to trigger auto-inc-dec pass */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer arithmetic */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int *data, int count) {
    const int *ptr = data;
    int sum = 0;
    
    /* Loop that should generate (reg + 0) addressing */
    for (int i = 0; i < count; i++) {
        /* Critical pattern: dereference pointer, then increment */
        sum += *ptr;    /* This should become (mem (plus (reg) (const_int 0))) */
        ptr += 1;       /* Post-increment */
        
        /* Use volatile to prevent loop optimization */
        if (dummy_volatile) break;
    }
    
    return sum;
}

/* Second function with write pattern */
__attribute__((noinline))
static void write_pattern(int *data, int count, int value) {
    int *ptr = data;
    
    for (int i = 0; i < count; i++) {
        /* Write with pointer dereference then increment */
        *ptr = value + i;
        ptr += 1;       /* Post-increment */
        
        if (dummy_volatile) break;
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make count non-constant for the compiler */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* First: read loop with pointer post-increment */
    int result = process_data(array, count);
    
    /* Second: write loop with pointer post-increment */
    write_pattern(array, count, 42);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (array[0] = %d)\n", result, array[0]);
    
    free(array);
    return 0;
}
