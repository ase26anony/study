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
static int process_data(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;
    
    /* Loop 1: Read using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* Critical pattern: dereference pointer before increment
         * Should generate (mem (plus (reg p) (const_int 0))) */
        sum += *p;
        p++;  /* Post-increment - should be recognized as auto-inc opportunity */
    }
    
    /* Reset pointer for second loop */
    p = ptr;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* Another opportunity for auto-inc recognition */
        *p = sum + i;
        p++;
    }
    
    /* Mix in volatile to prevent optimization */
    sum += dummy_volatile;
    
    return sum;
}

/* Another variation that might better produce (reg + 0) pattern */
__attribute__((noinline))
static int process_data_alt(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;
    
    /* Single statement with post-increment */
    for (int i = 0; i < count; i++) {
        /* Using *p++ directly might produce cleaner pattern */
        sum += *p++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command line argument to make count non-constant */
    int count = 10;
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 10;
    }
    
    /* Allocate and initialize array */
    int *data = (int *)malloc(count * sizeof(int));
    if (!data) return 1;
    
    for (int i = 0; i < count; i++) {
        data[i] = i * 2;
    }
    
    /* Call the processing functions */
    int result1 = process_data(data, count);
    int result2 = process_data_alt(data, count);
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %d, Result2: %d\n", result1, result2);
    
    /* Also use the modified array */
    int check = 0;
    for (int i = 0; i < count; i++) {
        check += data[i];
    }
    printf("Array checksum: %d\n", check);
    
    free(data);
    return 0;
}
