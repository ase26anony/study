/* auto_inc_trigger.c - Trigger auto-inc/dec recognition for (reg + 0) pattern */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer arithmetic */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int *data, int count) {
    const int *p = data;
    int sum = 0;
    int i;
    
    /* Loop 1: Read using pointer post-increment - aiming for (reg + 0) pattern */
    for (i = 0; i < count; i++) {
        /* Critical pattern: dereference pointer, then increment
         * Should generate: (mem (plus (reg p) (const_int 0))) */
        sum += *p;  /* This may become (mem (reg p)) or (mem (plus (reg p) (const_int 0))) */
        p++;        /* Post-increment */
        
        /* Use volatile to prevent loop optimization */
        if (dummy_volatile) break;
    }
    
    /* Reset pointer for second loop */
    p = data;
    
    /* Loop 2: Another pattern - write using pointer post-increment */
    for (i = 0; i < count; i++) {
        /* Cast away const for demonstration */
        int *q = (int *)p;
        /* Force memory write pattern */
        *q = *q + 1;
        p++;
        
        if (dummy_volatile) break;
    }
    
    return sum;
}

/* Alternative function with different pattern */
__attribute__((noinline))
static int process_data_alt(int *data, int count) {
    int *ptr = data;
    int total = 0;
    
    while (count-- > 0) {
        /* Direct pointer dereference - good candidate for (reg + 0) */
        total += *ptr;
        
        /* Explicit increment after use - post-increment pattern */
        ptr = ptr + 1;  /* Could be optimized to ptr++ */
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    int size = 100;
    int *array;
    int result1, result2;
    
    /* Use command line argument to prevent constant propagation */
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate and initialize array */
    array = (int *)malloc(size * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Call processing functions */
    result1 = process_data(array, size);
    result2 = process_data_alt(array, size);
    
    /* Print results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    
    /* Use array to prevent optimization */
    printf("First element: %d\n", array[0]);
    
    free(array);
    return 0;
}
