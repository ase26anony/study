/* auto_inc_test.c - Test program for auto-inc-dec pass coverage */

#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int *data, int count) {
    const volatile int *p = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* This should generate (reg + 0) pattern */
        sum += *p;
        p++;  /* Post-increment */
    }
    
    /* Reset pointer for second loop */
    p = data;
    
    /* Loop 2: Write using pointer post-increment */
    volatile int *wp = (volatile int *)p;
    for (int i = 0; i < count; i++) {
        /* Force memory write pattern */
        *wp = i;
        wp++;  /* Post-increment */
    }
    
    return sum;
}

/* Another variation with different pointer usage */
__attribute__((noinline))
static int process_data_alt(volatile int *data, int count) {
    volatile int *p = data;
    int sum = 0;
    
    /* Mix of read and write with pointer arithmetic */
    for (int i = 0; i < count; i++) {
        /* Read current value */
        sum += *p;
        
        /* Write modified value */
        *p = sum & 0xFF;
        
        /* Increment pointer - crucial for auto-inc pattern */
        p = p + 1;  /* Alternative form of p++ */
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use argv to prevent compile-time optimization */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    volatile int *data = (volatile int *)malloc(count * sizeof(int));
    if (!data) return 1;
    
    for (int i = 0; i < count; i++) {
        data[i] = i * 2;
    }
    
    /* Call processing functions */
    int result1 = process_data(data, count);
    int result2 = process_data_alt((volatile int *)data, count);
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %d, Result2: %d\n", result1, result2);
    
    /* Verify by reading back */
    int verify = 0;
    volatile int *p = data;
    for (int i = 0; i < count; i++) {
        verify += *p;
        p++;
    }
    printf("Verify: %d\n", verify);
    
    free((void *)data);
    return 0;
}
