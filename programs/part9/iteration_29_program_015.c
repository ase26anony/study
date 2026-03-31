/* auto_inc_dec_test.c
 * Designed to trigger auto-inc-dec pass lines 1352-1358
 * Compile with: gcc -O2 -march=armv7-a -fdump-rtl-auto_inc_dec -S auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inline function to isolate the critical loop */
int __attribute__((noinline)) process_data(const int* data, int count) {
    const int* p = data;
    int sum = 0;
    
    /* Loop with pointer dereference and post-increment
     * Should generate (reg + 0) address pattern */
    for (int i = 0; i < count; i++) {
        sum += *p;  /* Critical: dereference p before increment */
        p += 1;     /* Post-increment - may become auto-inc in RTL */
    }
    
    return sum;
}

/* Second non-inline function with write pattern */
void __attribute__((noinline)) write_data(int* dest, const int* src, int count) {
    int* d = dest;
    const int* s = src;
    
    /* Similar pattern but for writing */
    for (int i = 0; i < count; i++) {
        *d = *s;    /* Dereference both pointers */
        d += 1;
        s += 1;
    }
}

/* Volatile version to force preservation of memory operations */
int __attribute__((noinline)) process_volatile(volatile int* data, int count) {
    volatile int* p = data;
    int sum = 0;
    
    /* Volatile prevents optimization of memory accesses */
    for (int i = 0; i < count; i++) {
        sum += *p;
        p += 1;
    }
    
    return sum;
}

int main(int argc, char* argv[]) {
    /* Use command line argument to prevent constant propagation */
    int count = (argc > 1) ? atoi(argv[1]) : 100;
    if (count <= 0) count = 100;
    
    /* Allocate and initialize array */
    int* array = (int*)malloc(count * sizeof(int));
    int* dest = (int*)malloc(count * sizeof(int));
    
    if (!array || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < count; i++) {
        array[i] = i + 1;
        dest[i] = 0;
    }
    
    /* Call the processing functions */
    int sum1 = process_data(array, count);
    write_data(dest, array, count);
    int sum2 = process_volatile(dest, count);
    
    /* Print results to prevent dead code elimination */
    printf("Sum1: %d, Sum2: %d\n", sum1, sum2);
    
    /* Verify the copy worked */
    int verify = 1;
    for (int i = 0; i < count; i++) {
        if (dest[i] != array[i]) {
            verify = 0;
            break;
        }
    }
    printf("Copy verified: %s\n", verify ? "YES" : "NO");
    
    free(array);
    free(dest);
    
    return 0;
}
