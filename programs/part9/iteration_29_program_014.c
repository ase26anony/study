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
int __attribute__((noinline)) process_data(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;  /* Local pointer that will be incremented */
    
    /* Loop accessing memory with pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* Critical access: *p where p will be (reg + 0) in RTL */
        sum += *p;
        /* Post-increment - should create auto-inc opportunity */
        p++;
        
        /* Add volatile side effect to prevent loop optimization */
        dummy_volatile = i;
    }
    
    return sum;
}

/* Second function with write pattern */
void __attribute__((noinline)) write_pattern(int *ptr, int count, int value) {
    int *p = ptr;
    
    for (int i = 0; i < count; i++) {
        /* Write with pointer dereference */
        *p = value + i;
        /* Post-increment */
        p++;
        
        /* Volatile to prevent optimization */
        dummy_volatile = *p;
    }
}

int main(int argc, char *argv[]) {
    /* Use command line argument to make count non-constant */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    
    if (count <= 0) count = 100;
    
    /* Dynamically allocate to avoid static analysis */
    int *data = (int*)malloc(count * sizeof(int));
    if (!data) return 1;
    
    /* Initialize array */
    for (int i = 0; i < count; i++) {
        data[i] = i + 1;
    }
    
    /* Call the critical function */
    int result = process_data(data, count);
    
    /* Also test write pattern */
    write_pattern(data, count, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("First element after write: %d\n", data[0]);
    
    free(data);
    return 0;
}
