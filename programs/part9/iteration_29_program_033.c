/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile pointer to prevent optimization of pointer arithmetic */
static volatile int *volatile_ptr;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int *data, int count) {
    const int *p = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment - should generate (reg + 0) */
    for (int i = 0; i < count; i++) {
        /* Direct pointer dereference before increment */
        sum += *p;      /* This should generate address: (p + 0) */
        p++;            /* Post-increment */
    }
    
    /* Reset pointer for second loop */
    p = data;
    
    /* Loop 2: Another pattern with separate increment */
    for (int i = 0; i < count; i++) {
        int val = *p;   /* Another (p + 0) pattern */
        sum += val * 2;
        p = p + 1;      /* Explicit pointer addition */
    }
    
    return sum;
}

/* Another non-inline function with volatile access */
__attribute__((noinline))
static int process_with_volatile(int *data, int count) {
    int *p = data;
    int sum = 0;
    
    /* Use volatile pointer to force memory access */
    volatile_ptr = p;
    
    for (int i = 0; i < count; i++) {
        /* Access through volatile pointer reference */
        sum += *volatile_ptr;
        volatile_ptr++;  /* Increment volatile pointer */
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time optimization */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Dynamically allocate to prevent static analysis */
    int *data = (int *)malloc(size * sizeof(int));
    if (!data) return 1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        data[i] = i + 1;
    }
    
    /* Call processing functions */
    int result1 = process_data(data, size);
    int result2 = process_with_volatile(data, size);
    
    /* Print results to prevent elimination */
    printf("Result1: %d, Result2: %d\n", result1, result2);
    
    free(data);
    return 0;
}
