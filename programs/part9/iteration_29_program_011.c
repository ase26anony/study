/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -c auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimization of pointer arithmetic */
static volatile int dummy_volatile;

/* Non-inline function to isolate the critical loop */
int __attribute__((noinline)) process_data(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;  /* Local pointer that will be incremented */
    
    /* Loop that should generate (reg + 0) addressing */
    for (int i = 0; i < count; i++) {
        /* Critical access: *p where p hasn't been incremented yet */
        sum += *p;      /* This should generate (mem (plus (reg p) (const_int 0))) */
        p += 1;         /* Post-increment */
        
        /* Add volatile side effect to prevent loop optimization */
        dummy_volatile = i;
    }
    
    return sum;
}

/* Second function with write pattern */
void __attribute__((noinline)) write_pattern(int *dest, int *src, int count) {
    int *d = dest;
    int *s = src;
    
    for (int i = 0; i < count; i++) {
        /* Both read and write with post-increment */
        *d = *s;        /* Should generate (mem (plus (reg d) (const_int 0))) */
        d += 1;
        s += 1;
        
        dummy_volatile = *d;  /* Prevent optimization */
    }
}

int main(int argc, char **argv) {
    /* Use command line argument to make count non-constant */
    int count = 100;
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 100;
    }
    
    /* Allocate and initialize array */
    int *array = malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Call the critical function */
    int result = process_data(array, count);
    
    /* Also test write pattern */
    int *array2 = malloc(count * sizeof(int));
    if (array2) {
        write_pattern(array2, array, count);
        free(array2);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    free(array);
    return 0;
}
