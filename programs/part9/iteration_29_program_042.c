/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass recognition of (reg + 0) pattern
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 * Or for x86: gcc -O2 -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer arithmetic */
static volatile int dummy_volatile = 0;

/* Non-inline function to isolate the critical loop */
__attribute__((noinline)) 
static int process_data(const int *ptr, int count) {
    int sum = 0;
    const int *p = ptr;  /* Local pointer that will be incremented */
    
    /* Loop accessing array with pointer post-increment
     * This should generate (reg + 0) address pattern */
    for (int i = 0; i < count; i++) {
        /* Critical access: *p where p is later incremented
         * The address calculation for *p may become (p + 0) */
        sum += *p;
        
        /* Post-increment - crucial for auto-inc-dec recognition */
        p++;
        
        /* Volatile operation to prevent loop optimization */
        dummy_volatile = i;
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
__attribute__((noinline))
static void write_data(int *ptr, int count, int value) {
    int *p = ptr;
    
    for (int i = 0; i < count; i++) {
        /* Write with pointer dereference, then increment */
        *p = value + i;
        p++;
        
        /* Prevent optimization */
        dummy_volatile = *p;
    }
}

int main(int argc, char *argv[]) {
    /* Use command line argument to make count non-constant for optimizer */
    int count = 10;
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 10;
    }
    
    /* Allocate and initialize array */
    int *data = (int*)malloc(count * sizeof(int));
    if (!data) return 1;
    
    for (int i = 0; i < count; i++) {
        data[i] = i + 1;
    }
    
    /* Process data with read loop */
    int sum = process_data(data, count);
    
    /* Write data with write loop */
    write_data(data, count, 100);
    
    /* Use results to prevent dead code elimination */
    printf("Sum: %d, First element: %d\n", sum, data[0]);
    
    /* Verify by recomputing sum */
    int verify_sum = 0;
    for (int i = 0; i < count; i++) {
        verify_sum += data[i];
    }
    printf("Verify sum: %d\n", verify_sum);
    
    free(data);
    return 0;
}
