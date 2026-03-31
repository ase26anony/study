/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass with zero-offset memory access
 * patterns, specifically targeting lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

/* Use noinline to prevent inlining and preserve loop structure */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ generates base+0 addressing */
    while (n--) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src generates base+0 addressing */
    while (n--) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ generates base+0 addressing */
    while (n--) {
        sum += *p++;  /* Should generate (mem (reg)) with offset 0 */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src + n - 1;
    
    /* Mixed patterns: post-increment and pre-decrement */
    while (n--) {
        *d++ = *s--;  /* Both should generate base+0 addressing */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(short *arr, short value, int n) {
    short *p = arr;
    
    /* Pattern: *p++ = value generates base+0 addressing */
    while (n--) {
        *p++ = value;  /* Should generate (mem (reg)) with offset 0 */
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    int backup[ARRAY_SIZE];
    int sum1, sum2;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
        backup[i] = src[i];
    }
    
    /* Test 1: Post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_increment(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            return -1;
        }
    }
    
    /* Test 2: Pre-decrement copy (reverse copy) */
    memset(dst, 0, sizeof(dst));
    copy_with_pre_decrement(dst + ARRAY_SIZE - 1, src, ARRAY_SIZE);
    
    /* Verify reverse copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[ARRAY_SIZE - 1 - i]) {
            return -2;
        }
    }
    
    /* Test 3: Sum with post-increment */
    sum1 = sum_with_post_increment(src, ARRAY_SIZE);
    
    /* Calculate expected sum */
    sum2 = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum2 += src[i];
    }
    
    if (sum1 != sum2) {
        return -3;
    }
    
    /* Test 4: Character array reversal */
    char char_src[ARRAY_SIZE];
    char char_dst[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        char_src[i] = (char)(i % 256);
    }
    
    reverse_with_dual_pointers(char_dst, char_src, ARRAY_SIZE);
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (char_dst[i] != char_src[ARRAY_SIZE - 1 - i]) {
            return -4;
        }
    }
    
    /* Test 5: Fill with post-increment */
    short short_arr[ARRAY_SIZE];
    short fill_value = 0x55AA;
    
    fill_with_post_increment(short_arr, fill_value, ARRAY_SIZE);
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (short_arr[i] != fill_value) {
            return -5;
        }
    }
    
    /* Restore source (ensures compiler can't optimize away initialization) */
    copy_with_post_increment(src, backup, ARRAY_SIZE);
    
    return 0; /* All tests passed */
}

int main(void) {
    int result = test_combined_patterns();
    
    if (result == 0) {
        printf("All auto-inc-dec patterns executed successfully\n");
        printf("Check RTL dumps for auto-inc-dec pass activity\n");
    } else {
        printf("Test failed with error code: %d\n", result);
    }
    
    return result;
}
