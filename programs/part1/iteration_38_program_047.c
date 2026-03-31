/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the zero-offset memory access pattern in find_inc()
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

/* Prevent inlining to ensure loops remain distinct for analysis */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset memory accesses */
    while (n--) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src creates zero-offset memory accesses */
    dst += n;
    src += n;
    while (n--) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    /* Pattern: sum += *ptr++ creates read with zero offset */
    int sum = 0;
    const int *ptr = arr;
    while (n--) {
        sum += *ptr++;  /* Memory read with zero offset */
    }
    return sum;
}

static void __attribute__((noinline))
fill_with_post_increment(int *arr, int value, int n) {
    /* Pattern: *ptr++ = value creates write with zero offset */
    int *ptr = arr;
    while (n--) {
        *ptr++ = value;  /* Memory write with zero offset */
    }
}

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    /* Complex pattern using both increment and decrement */
    int *start = arr;
    int *end = arr + n - 1;
    
    while (start < end) {
        /* Both accesses have zero offset */
        int temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

/* Test function that combines multiple patterns */
static void __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    int i;
    
    /* Initialize source with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Test 1: Simple post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_increment(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("Error in copy_with_post_increment at index %d\n", i);
            return;
        }
    }
    
    /* Test 2: Pre-decrement reverse copy */
    memset(dst, 0, sizeof(dst));
    copy_with_pre_decrement(dst, src, ARRAY_SIZE);
    
    /* Verify reverse copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[ARRAY_SIZE - 1 - i]) {
            printf("Error in copy_with_pre_decrement at index %d\n", i);
            return;
        }
    }
    
    /* Test 3: Sum calculation */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    int expected_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (sum != expected_sum) {
        printf("Error in sum_with_post_increment: %d != %d\n", 
               sum, expected_sum);
        return;
    }
    
    /* Test 4: Array fill */
    fill_with_post_increment(dst, 42, ARRAY_SIZE);
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != 42) {
            printf("Error in fill_with_post_increment at index %d\n", i);
            return;
        }
    }
    
    /* Test 5: In-place reversal */
    for (i = 0; i < ARRAY_SIZE; i++) {
        dst[i] = i;
    }
    reverse_with_dual_pointers(dst, ARRAY_SIZE);
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != ARRAY_SIZE - 1 - i) {
            printf("Error in reverse_with_dual_pointers at index %d\n", i);
            return;
        }
    }
    
    printf("All auto-inc-dec patterns executed successfully!\n");
}

int main(void) {
    /* Call the test function multiple times to ensure
     * the compiler doesn't optimize away the loops */
    for (int i = 0; i < 3; i++) {
        test_combined_patterns();
    }
    
    return 0;
}
