/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 * Or for x86: gcc -O2 -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to isolate the loop for RTL analysis */
__attribute__((noinline)) 
static int process_data(const volatile int* data, int count) {
    const int* ptr = data;
    int sum = 0;
    
    /* Loop 1: Read using pointer post-increment
     * This should generate (reg + 0) pattern for auto-inc-dec pass */
    for (int i = 0; i < count; i++) {
        /* Direct pointer dereference - compiler may generate (ptr + 0) */
        sum += *ptr;
        /* Post-increment - crucial for auto-inc-dec recognition */
        ptr++;
    }
    
    return sum;
}

/* Second function with write pattern */
__attribute__((noinline))
static void modify_data(volatile int* data, int count, int value) {
    int* ptr = (int*)data;
    
    /* Loop 2: Write using pointer post-increment */
    for (int i = 0; i < count; i++) {
        /* Write through pointer */
        *ptr = value + i;
        /* Post-increment */
        ptr++;
    }
}

int main(int argc, char* argv[]) {
    /* Use command line argument to prevent constant propagation */
    int count = 100;
    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 100;
    }
    
    /* Allocate and initialize array */
    int* array = (int*)malloc(count * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
    }
    
    /* Process data - this function contains the critical loop */
    int result = process_data(array, count);
    
    /* Modify data - additional pattern */
    modify_data(array, count, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Verify by computing again */
    int verify = 0;
    for (int i = 0; i < count; i++) {
        verify += array[i];
    }
    printf("Verify: %d\n", verify);
    
    free(array);
    return 0;
}
