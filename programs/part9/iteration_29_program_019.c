/* auto_inc_test.c - Test program for auto-increment/decrement recognition */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer arithmetic */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int *data, int count) {
    const int *ptr = data;
    int sum = 0;
    int i;
    
    /* Loop 1: Read using pointer post-increment */
    /* This should generate (reg + 0) pattern in RTL */
    for (i = 0; i < count; i++) {
        /* Direct pointer dereference followed by increment */
        sum += *ptr;    /* Address: (ptr + 0) */
        ptr += 1;       /* Post-increment */
        
        /* Use volatile to prevent loop optimizations */
        if (dummy_volatile) break;
    }
    
    /* Reset pointer for second loop */
    ptr = data;
    
    /* Loop 2: Write using pointer post-increment */
    int *wptr = (int *)data;  /* Cast away const for writing */
    for (i = 0; i < count; i++) {
        /* Write pattern with post-increment */
        *wptr = *wptr + 1;    /* Read and write same location */
        wptr += 1;            /* Post-increment */
    }
    
    return sum;
}

/* Another non-inline function with different pattern */
__attribute__((noinline))
static int process_data_alt(int *data, int count) {
    int *p = data;
    int total = 0;
    
    /* While loop with pointer dereference */
    while (count-- > 0) {
        total ^= *p;    /* Use XOR to prevent simple optimization */
        p++;            /* Post-increment */
        
        /* Mix with volatile to block optimization */
        if (dummy_volatile) p = data;
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    /* Use command line argument to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Dynamically allocate to avoid stack array optimizations */
    int *array = malloc(size * sizeof(int));
    if (!array) return 1;
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Call processing functions */
    int result1 = process_data(array, size);
    int result2 = process_data_alt(array, size);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Use array values to prevent optimization */
    int check = array[size/2] + dummy_volatile;
    printf("Check value: %d\n", check);
    
    free(array);
    return 0;
}
