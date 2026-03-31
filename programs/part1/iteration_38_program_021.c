/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-inc-dec pass
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 * where reg1_val = 0 for memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 256
#define ITERATIONS 1000

/* Prevent inlining to ensure loops remain intact */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates base+0 addressing */
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates base+0 read access */
    while (n-- > 0) {
        sum += *p++;  /* Memory read with zero offset */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src + n - 1;
    
    /* Mixed patterns: post-increment and pre-decrement */
    while (n-- > 0) {
        *d++ = *s--;  /* Should create two (mem (reg)) patterns */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(volatile int *arr, int n, int value) {
    volatile int *p = arr;
    
    /* Volatile prevents dead code elimination */
    while (n-- > 0) {
        *p++ = value;  /* Memory write with zero offset */
    }
}

/* Test different data types to explore various patterns */
static void __attribute__((noinline))
copy_bytes_with_inc_dec(unsigned char *dst, const unsigned char *src, int n) {
    /* Simple byte copy with post-increment */
    while (n-- > 0) {
        *dst++ = *src++;
    }
}

int main(void) {
    int src_array[SIZE];
    int dst_array[SIZE];
    char src_str[SIZE];
    char dst_str[SIZE];
    volatile int volatile_array[SIZE];
    
    int i;
    
    /* Initialize source data */
    for (i = 0; i < SIZE; i++) {
        src_array[i] = i * 3;
        src_str[i] = 'A' + (i % 26);
    }
    
    /* Clear destination arrays */
    memset(dst_array, 0, sizeof(dst_array));
    memset(dst_str, 0, sizeof(dst_str));
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test 1: Post-increment copy (most common pattern) */
    copy_with_post_increment(dst_array, src_array, SIZE);
    
    /* Verify copy */
    for (i = 0; i < SIZE; i++) {
        if (dst_array[i] != src_array[i]) {
            printf("FAIL: Post-increment copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Post-increment copy\n");
    
    /* Reset and test pre-decrement */
    memset(dst_array, 0, sizeof(dst_array));
    copy_with_pre_decrement(dst_array, src_array, SIZE);
    
    /* Verify reverse copy */
    for (i = 0; i < SIZE; i++) {
        if (dst_array[i] != src_array[i]) {
            printf("FAIL: Pre-decrement copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Pre-decrement copy\n");
    
    /* Test sum calculation */
    int sum = sum_with_post_increment(src_array, SIZE);
    int expected_sum = 0;
    for (i = 0; i < SIZE; i++) {
        expected_sum += src_array[i];
    }
    
    if (sum != expected_sum) {
        printf("FAIL: Sum calculation mismatch: %d vs %d\n", sum, expected_sum);
        return 1;
    }
    printf("PASS: Sum calculation with post-increment\n");
    
    /* Test reverse copy with mixed patterns */
    reverse_with_dual_pointers(dst_str, src_str, SIZE);
    
    /* Verify reverse */
    for (i = 0; i < SIZE; i++) {
        if (dst_str[i] != src_str[SIZE - 1 - i]) {
            printf("FAIL: Reverse copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Reverse copy with mixed patterns\n");
    
    /* Test volatile write pattern */
    fill_with_post_increment(volatile_array, SIZE, 0x55AA);
    
    /* Test byte copy */
    unsigned char byte_src[SIZE];
    unsigned char byte_dst[SIZE];
    
    for (i = 0; i < SIZE; i++) {
        byte_src[i] = (unsigned char)(i & 0xFF);
    }
    
    copy_bytes_with_inc_dec(byte_dst, byte_src, SIZE);
    
    if (memcmp(byte_src, byte_dst, SIZE) != 0) {
        printf("FAIL: Byte copy mismatch\n");
        return 1;
    }
    printf("PASS: Byte copy with increment\n");
    
    /* Run multiple iterations to increase chance of optimization */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        copy_with_post_increment(dst_array, src_array, SIZE);
        sum_with_post_increment(src_array, SIZE);
    }
    
    printf("\nAll tests passed! The auto-inc-dec pass should have processed\n");
    printf("the zero-offset memory access patterns in the loops.\n");
    
    return 0;
}
