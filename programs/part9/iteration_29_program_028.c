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
     * This should generate (reg + 0) pattern for the load address */
    for (int i = 0; i < count; i++) {
        /* Direct pointer dereference before increment */
        sum += *p;
        p += 1;  /* Post-increment equivalent to p++ */
    }
    
    /* Reset pointer for second loop */
    p = ptr;
    
    /* Loop 2: Write using pointer post-increment
     * Increases chance of hitting the pattern */
    for (int i = 0; i < count; i++) {
        /* Store through pointer before increment */
        *p = sum + i;
        p += 1;  /* Post-increment */
    }
    
    /* Use volatile to prevent dead code elimination */
    dummy_volatile = sum;
    
    return sum;
}

/* Alternative version with more explicit pattern */
__attribute__((noinline))
static int process_data_alt(int *ptr, int count) {
    int sum = 0;
    int *p = ptr;
    
    /* This loop structure often produces (reg + 0) in RTL */
    while (count-- > 0) {
        /* The key: dereference p without any offset */
        int val = *p;
        sum += val;
        /* Increment after use - post-increment pattern */
        p = p + 1;
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
    int *data = malloc(count * sizeof(int));
    if (!data) return 1;
    
    for (int i = 0; i < count; i++) {
        data[i] = i + 1;
    }
    
    /* Call the processing function */
    int result = process_data(data, count);
    
    /* Also call alternative version */
    int result2 = process_data_alt(data, count / 2);
    
    /* Print results to prevent optimization */
    printf("Result 1: %d\n", result);
    printf("Result 2: %d\n", result2);
    
    /* Use the data to prevent dead store elimination */
    for (int i = 0; i < count; i++) {
        dummy_volatile += data[i];
    }
    
    free(data);
    return 0;
}
