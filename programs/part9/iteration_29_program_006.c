/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer arithmetic */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;
    
    /* Loop 1: Read using pointer post-increment
     * This should generate (reg + 0) pattern */
    for (int i = 0; i < count; i++) {
        /* Force use of pointer as lvalue before increment
         * The *p access should become (mem (plus (reg p) (const_int 0))) */
        sum += *p;
        p += 1;  /* Post-increment - may become auto-inc after transformation */
    }
    
    /* Reset pointer for second loop */
    p = ptr;
    
    /* Loop 2: Write using pointer post-increment
     * Increases chance of hitting the pattern */
    for (int i = 0; i < count; i++) {
        *p = sum + i;
        p += 1;
    }
    
    /* Mix in volatile to prevent over-optimization */
    sum += dummy_volatile;
    
    return sum;
}

/* Another variation that directly uses pointer dereference */
__attribute__((noinline))
static int process_data_direct(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;
    
    /* Direct pointer access in loop - good candidate for (reg + 0) */
    while (count-- > 0) {
        /* The critical pattern: dereference pointer, then increment */
        int val = *p;      /* Should be (mem (plus (reg p) (const_int 0))) */
        sum += val;
        p = p + 1;         /* Simple pointer arithmetic */
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use argc to make count non-constant to optimizer */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int *data = malloc(count * sizeof(int));
    if (!data) return 1;
    
    for (int i = 0; i < count; i++) {
        data[i] = i + 1;
    }
    
    /* Call processing functions */
    int result1 = process_data(data, count);
    int result2 = process_data_direct(data, count);
    
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
