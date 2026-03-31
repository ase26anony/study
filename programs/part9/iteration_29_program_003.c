/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int* data, int count) {
    const int* p = data;  /* Pointer that will be incremented */
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment pattern */
    for (int i = 0; i < count; i++) {
        /* Critical pattern: *p where p is later incremented
         * Should generate (reg + 0) address pattern */
        sum += *p;    /* Memory access at (p + 0) */
        p++;          /* Post-increment */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int* data, int count, int value) {
    int* q = (int*)data;
    
    /* Loop 2: Write using pointer post-increment pattern */
    for (int i = 0; i < count; i++) {
        *q = value + i;  /* Memory write at (q + 0) */
        q++;             /* Post-increment */
    }
}

int main(int argc, char* argv[]) {
    /* Use command line argument to prevent constant propagation */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int* array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Process data - this should trigger the auto-inc-dec logic */
    int result = process_data(array, count);
    
    /* Modify data - additional pattern to increase coverage chance */
    modify_data(array, count, 10);
    
    /* Use results to prevent optimization */
    printf("Result: %d, First element after modify: %d\n", 
           result, array[0]);
    
    free(array);
    return 0;
}
