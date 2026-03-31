/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass recognition of (reg + 0) pattern
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -c auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of memory accesses */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;
    
    /* Loop 1: Read using pointer post-increment
     * This should generate (reg + 0) pattern for the memory address */
    for (int i = 0; i < count; i++) {
        /* Direct pointer dereference - compiler may see this as (p + 0) */
        sum += *p;
        /* Post-increment - critical for auto-inc-dec recognition */
        p++;
        
        /* Add volatile side effect to prevent loop optimization */
        if (dummy_volatile) break;
    }
    
    /* Reset pointer for second loop */
    p = ptr;
    
    /* Loop 2: Write using pointer post-increment
     * Increases chance of hitting the target code block */
    for (int i = 0; i < count; i++) {
        /* Write operation with pointer dereference */
        *p = *p + 1;
        /* Post-increment */
        p++;
        
        /* Another volatile guard */
        if (dummy_volatile) break;
    }
    
    return sum;
}

/* Another variation with different pointer usage pattern */
__attribute__((noinline))
static int process_data_alt(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;
    int i = 0;
    
    /* While loop with pointer dereference before increment */
    while (i < count) {
        /* The address calculation here may become (p + 0) */
        int val = *p;
        sum += val;
        p = p + 1;  /* Explicit pointer addition */
        i++;
        
        if (dummy_volatile) break;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command line argument to make count non-constant */
    int count = 100;
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 100;
    }
    
    /* Allocate and initialize array */
    int *data = (int*)malloc(count * sizeof(int));
    if (!data) return 1;
    
    for (int i = 0; i < count; i++) {
        data[i] = i + 1;
    }
    
    /* Call the processing functions */
    int result1 = process_data(data, count);
    int result2 = process_data_alt(data, count);
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %d, Result2: %d\n", result1, result2);
    
    /* Also use the modified array */
    int verify = 0;
    for (int i = 0; i < count; i++) {
        verify += data[i];
    }
    printf("Array sum: %d\n", verify);
    
    free(data);
    return 0;
}
