/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass with zero-offset memory access
 * patterns, specifically targeting lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 256

/* Prevent inlining to ensure loops remain distinct for analysis */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates zero-offset accesses */
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates zero-offset read */
    while (n-- > 0) {
        sum += *p++;  /* Memory access with pointer, no explicit offset */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src + n - 1;
    
    /* Mixed patterns: post-increment on dst, pre-decrement on src */
    while (n-- > 0) {
        *d++ = *s--;  /* Both accesses should be zero-offset */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(volatile int *buf, int value, int n) {
    volatile int *p = buf;
    
    /* Pattern: *p++ = value with volatile to prevent elimination */
    while (n-- > 0) {
        *p++ = value;  /* Zero-offset write */
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[SIZE];
    int dst[SIZE];
    int sum, checksum = 0;
    
    /* Initialize source with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
        checksum += src[i];
    }
    
    /* Test 1: Simple post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            return -1;
        }
    }
    
    /* Test 2: Pre-decrement reverse copy */
    int src_rev[SIZE];
    copy_with_pre_decrement(src_rev, src, SIZE);
    
    /* Verify reverse copy */
    for (int i = 0; i < SIZE; i++) {
        if (src_rev[i] != src[SIZE - 1 - i]) {
            return -2;
        }
    }
    
    /* Test 3: Sum with post-increment */
    sum = sum_with_post_increment(src, SIZE);
    if (sum != checksum) {
        return -3;
    }
    
    /* Test 4: Char array reverse with dual pointers */
    char str_src[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char str_dst[sizeof(str_src)];
    int len = strlen(str_src);
    
    reverse_with_dual_pointers(str_dst, str_src, len);
    str_dst[len] = '\0';
    
    /* Verify string reversal */
    for (int i = 0; i < len; i++) {
        if (str_dst[i] != str_src[len - 1 - i]) {
            return -4;
        }
    }
    
    /* Test 5: Volatile write pattern */
    volatile int volatile_buf[SIZE];
    fill_with_post_increment(volatile_buf, 0x55AA, SIZE);
    
    /* Verify volatile writes */
    for (int i = 0; i < SIZE; i++) {
        if (volatile_buf[i] != 0x55AA) {
            return -5;
        }
    }
    
    return 0; /* All tests passed */
}

int main(void) {
    int result = test_combined_patterns();
    
    if (result == 0) {
        printf("All auto-inc-dec patterns executed successfully\n");
        printf("Check RTL dump for auto-inc-dec pass transformations\n");
    } else {
        printf("Test failed with error code: %d\n", result);
    }
    
    return result;
}
