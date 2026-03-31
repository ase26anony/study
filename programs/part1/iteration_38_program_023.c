/* test_auto_inc_dec.c
 * This program creates memory access patterns that should trigger
 * GCC's auto-increment/decrement optimization pass, specifically
 * the zero-offset pattern in find_inc().
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 256
#define CHECKSUM_SEED 0x12345678

/* Global volatile to prevent dead code elimination */
volatile int global_checksum = 0;

/* Noinline to ensure function boundaries are preserved */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ 
     * Should generate: (mem (reg dst)) with offset 0
     *                 (mem (reg src)) with offset 0
     */
    while (n-- > 0) {
        *dst++ = *src++;
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src
     * Should generate: (mem (pre_dec (reg dst))) with offset 0
     *                 (mem (pre_dec (reg src))) with offset 0
     */
    dst += n;
    src += n;
    while (n-- > 0) {
        *--dst = *--src;
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    /* Pattern: sum += *ptr++
     * Should generate: (mem (reg ptr)) with offset 0
     */
    int sum = 0;
    const int *ptr = arr;
    while (n-- > 0) {
        sum += *ptr++;
    }
    return sum;
}

static int __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    /* Pattern: *p1++ = *--p2
     * Complex pattern that might trigger multiple cases
     */
    int *p1 = arr;
    int *p2 = arr + n;
    int temp_sum = 0;
    
    while (p1 < p2) {
        *p1++ = *--p2;
        temp_sum += *p1;
    }
    return temp_sum;
}

static void __attribute__((noinline))
byte_copy_with_inc_dec(char *dst, const char *src, int n) {
    /* Using char pointers for different data width */
    while (n-- > 0) {
        *dst++ = *src++;
    }
}

static void __attribute__((noinline))
fill_with_post_increment(int *arr, int value, int n) {
    /* Pattern: *ptr++ = value
     * Simple store with post-increment
     */
    int *ptr = arr;
    while (n-- > 0) {
        *ptr++ = value;
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[SIZE];
    int dst[SIZE];
    int checksum = CHECKSUM_SEED;
    
    /* Initialize source with predictable values */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Test 1: Simple post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: copy_with_post_increment at index %d\n", i);
            return -1;
        }
    }
    checksum += dst[SIZE/2];
    
    /* Test 2: Pre-decrement reverse copy */
    memset(dst, 0, sizeof(dst));
    copy_with_pre_decrement(dst, src, SIZE);
    
    /* Verify reverse copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[SIZE - 1 - i]) {
            printf("FAIL: copy_with_pre_decrement at index %d\n", i);
            return -1;
        }
    }
    checksum += dst[SIZE/4];
    
    /* Test 3: Sum calculation with post-increment */
    int sum = sum_with_post_increment(src, SIZE);
    int expected_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        expected_sum += src[i];
    }
    if (sum != expected_sum) {
        printf("FAIL: sum_with_post_increment: %d != %d\n", sum, expected_sum);
        return -1;
    }
    checksum += sum;
    
    /* Test 4: Byte copy (different data width) */
    char src_bytes[SIZE];
    char dst_bytes[SIZE];
    for (int i = 0; i < SIZE; i++) {
        src_bytes[i] = (char)(i & 0xFF);
    }
    byte_copy_with_inc_dec(dst_bytes, src_bytes, SIZE);
    if (memcmp(src_bytes, dst_bytes, SIZE) != 0) {
        printf("FAIL: byte_copy_with_inc_dec\n");
        return -1;
    }
    checksum += dst_bytes[SIZE/2];
    
    /* Test 5: Fill with post-increment */
    int fill_value = 0xABCD;
    fill_with_post_increment(dst, fill_value, SIZE);
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != fill_value) {
            printf("FAIL: fill_with_post_increment at index %d\n", i);
            return -1;
        }
    }
    checksum += dst[SIZE/3];
    
    /* Store to global volatile to prevent optimization */
    global_checksum = checksum;
    
    return checksum;
}

int main(void) {
    printf("Testing auto-inc-dec patterns...\n");
    
    int result = test_combined_patterns();
    
    if (result < 0) {
        printf("Test failed!\n");
        return 1;
    }
    
    printf("Test passed with checksum: %d\n", result);
    printf("Global checksum: %d\n", global_checksum);
    
    /* Additional simple direct test */
    {
        int a[10], b[10];
        for (int i = 0; i < 10; i++) a[i] = i * 2;
        
        /* This should generate clean post-increment pattern */
        int *src = a;
        int *dst = b;
        for (int i = 0; i < 10; i++) {
            *dst++ = *src++;
        }
        
        /* And pre-decrement pattern */
        src = a + 10;
        dst = b + 10;
        for (int i = 0; i < 10; i++) {
            *--dst = *--src;
        }
        
        printf("Simple test completed\n");
    }
    
    return 0;
}
