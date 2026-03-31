/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass with zero-offset memory access
 * patterns (reg1_val = 0) in find_inc() function.
 */

#include <stdio.h>
#include <string.h>

/* Prevent inlining to ensure loops remain distinct for analysis */
static void __attribute__((noinline)) 
copy_forward_postinc(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ - should generate base+0 offset */
    while (n-- > 0) {
        *dst++ = *src++;
    }
}

static void __attribute__((noinline))
copy_backward_predec(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src - should generate base+0 offset */
    dst += n;
    src += n;
    while (n-- > 0) {
        *--dst = *--src;
    }
}

static int __attribute__((noinline))
sum_array_postinc(const int *arr, int n) {
    /* Pattern: sum += *ptr++ - read with post-increment */
    int sum = 0;
    const int *ptr = arr;
    while (n-- > 0) {
        sum += *ptr++;
    }
    return sum;
}

static void __attribute__((noinline))
fill_array_predec(int *arr, int n, int value) {
    /* Pattern: *--ptr = value - write with pre-decrement */
    int *ptr = arr + n;
    while (n-- > 0) {
        *--ptr = value + n; /* Different values to prevent optimization */
    }
}

static void __attribute__((noinline))
reverse_array_pointers(char *dst, const char *src, int n) {
    /* Mixed pattern: *dst++ = *--src_end */
    const char *src_end = src + n;
    while (n-- > 0) {
        *dst++ = *--src_end;
    }
}

/* Test with different data types to explore various patterns */
static int __attribute__((noinline))
process_shorts(short *dst, const short *src, int n) {
    int sum = 0;
    while (n-- > 0) {
        *dst++ = *src++;
        sum += *dst; /* Use result to prevent dead code elimination */
    }
    return sum;
}

#define ARRAY_SIZE 256

int main(void) {
    /* Source arrays with known patterns */
    int src_int[ARRAY_SIZE];
    int dst_int[ARRAY_SIZE];
    char src_char[ARRAY_SIZE];
    char dst_char[ARRAY_SIZE];
    short src_short[ARRAY_SIZE];
    short dst_short[ARRAY_SIZE];
    
    /* Initialize source arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src_int[i] = i * 3 + 1;
        src_char[i] = (char)(i % 128);
        src_short[i] = (short)(i * 2);
    }
    
    /* Clear destination arrays */
    memset(dst_int, 0, sizeof(dst_int));
    memset(dst_char, 0, sizeof(dst_char));
    memset(dst_short, 0, sizeof(dst_short));
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test 1: Forward copy with post-increment */
    copy_forward_postinc(dst_int, src_int, ARRAY_SIZE);
    
    /* Verify */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst_int[i] != src_int[i]) {
            printf("FAIL: copy_forward_postinc at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: copy_forward_postinc\n");
    
    /* Test 2: Backward copy with pre-decrement */
    copy_backward_predec(dst_int, src_int, ARRAY_SIZE);
    
    /* Verify (array should be reversed) */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst_int[i] != src_int[ARRAY_SIZE - 1 - i]) {
            printf("FAIL: copy_backward_predec at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: copy_backward_predec\n");
    
    /* Test 3: Sum with post-increment */
    int sum = sum_array_postinc(src_int, ARRAY_SIZE);
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src_int[i];
    }
    if (sum != expected_sum) {
        printf("FAIL: sum_array_postinc: %d != %d\n", sum, expected_sum);
        return 1;
    }
    printf("PASS: sum_array_postinc\n");
    
    /* Test 4: Fill with pre-decrement */
    fill_array_predec(dst_int, ARRAY_SIZE, 100);
    
    /* Verify fill pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst_int[i] != 100 + (ARRAY_SIZE - 1 - i)) {
            printf("FAIL: fill_array_predec at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: fill_array_predec\n");
    
    /* Test 5: Reverse with mixed pointers */
    reverse_array_pointers(dst_char, src_char, ARRAY_SIZE);
    
    /* Verify reversal */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst_char[i] != src_char[ARRAY_SIZE - 1 - i]) {
            printf("FAIL: reverse_array_pointers at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: reverse_array_pointers\n");
    
    /* Test 6: Different data type (short) */
    int short_sum = process_shorts(dst_short, src_short, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst_short[i] != src_short[i]) {
            printf("FAIL: process_shorts at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: process_shorts (sum = %d)\n", short_sum);
    
    printf("\nAll tests passed! The auto-inc-dec pass should have seen\n");
    printf("memory accesses with zero offset (reg1_val = 0) patterns.\n");
    
    return 0;
}
