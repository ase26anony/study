/* auto_inc_dec_test.c - Test program for auto-inc-dec pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const volatile int* data, int count) {
    const volatile int* p = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* This should generate (reg + 0) pattern */
        sum += *p;
        p++;  /* Post-increment - key for auto-inc-dec recognition */
    }
    
    /* Reset pointer for second loop */
    p = data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* Cast away volatile for writing */
        *(int*)p = i;
        p++;  /* Another post-increment opportunity */
    }
    
    return sum;
}

/* Another non-inline function with different pattern */
__attribute__((noinline))
static int process_data_alt(volatile int* data, int count) {
    volatile int* ptr = data;
    int result = 0;
    
    /* Mixed read/write with pointer arithmetic */
    while (count-- > 0) {
        result += *ptr;      /* Read current */
        *ptr = result;       /* Write back */
        ptr += 1;           /* Increment by 1 */
    }
    
    return result;
}

int main(int argc, char** argv) {
    /* Use argc to make count non-constant for optimizer */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    volatile int* array = (volatile int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i * 2;
    }
    
    /* Call processing functions - prevent inlining */
    int sum1 = process_data(array, count);
    int sum2 = process_data_alt(array, count);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %d\n", sum1);
    printf("Result 2: %d\n", sum2);
    
    /* Verify by recalculating */
    int verify = 0;
    for (int i = 0; i < count; i++) {
        verify += array[i];
    }
    printf("Verify: %d\n", verify);
    
    free((void*)array);
    return 0;
}
