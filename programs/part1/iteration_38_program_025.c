/* test_auto_inc_dec.c
 * Program to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns.
 * Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c test_auto_inc_dec.c
 */

#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Prevent inlining to ensure loops remain distinct */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ - should generate base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;
    }
}

static void __attribute__((noinline)) 
reverse_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src - should generate base+0 addressing */
    dst += n;
    src += n;
    while (n-- > 0) {
        *--dst = *--src;
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    /* Pattern: sum += *arr++ - read with post-increment */
    int sum = 0;
    while (n-- > 0) {
        sum += *arr++;
    }
    return sum;
}

static void __attribute__((noinline)) 
fill_with_pre_decrement(int *arr, int n, int value) {
    /* Pattern: *--arr = value - write with pre-decrement */
    arr += n;
    while (n-- > 0) {
        *--arr = value;
    }
}

/* Test char pointers as well - different size might trigger different patterns */
static void __attribute__((noinline))
char_copy_with_inc(char *dst, const char *src, int n) {
    /* Pattern: *dst++ = *src++ with char type */
    while (n-- > 0) {
        *dst++ = *src++;
    }
}

int main(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    char src_char[ARRAY_SIZE];
    char dst_char[ARRAY_SIZE];
    
    /* Initialize source arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
        src_char[i] = (char)(i % 256);
    }
    
    /* Test 1: Post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_increment(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Post-increment copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Post-increment copy\n");
    
    /* Test 2: Pre-decrement reverse */
    memset(dst, 0, sizeof(dst));
    reverse_with_pre_decrement(dst, src, ARRAY_SIZE);
    
    /* Verify reverse copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[ARRAY_SIZE - 1 - i]) {
            printf("FAIL: Pre-decrement reverse mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Pre-decrement reverse\n");
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    if (sum != expected_sum) {
        printf("FAIL: Sum mismatch: got %d, expected %d\n", sum, expected_sum);
        return 1;
    }
    printf("PASS: Sum with post-increment = %d\n", sum);
    
    /* Test 4: Fill with pre-decrement */
    memset(dst, 0, sizeof(dst));
    fill_with_pre_decrement(dst, ARRAY_SIZE, 42);
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != 42) {
            printf("FAIL: Fill with pre-decrement mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Fill with pre-decrement\n");
    
    /* Test 5: Char copy with increment */
    memset(dst_char, 0, sizeof(dst_char));
    char_copy_with_inc(dst_char, src_char, ARRAY_SIZE);
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst_char[i] != src_char[i]) {
            printf("FAIL: Char copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Char copy with increment\n");
    
    printf("\nAll tests passed! The auto-inc-dec pass should have processed the loops.\n");
    printf("Check the dump file (test_auto_inc_dec.c.*.auto-inc-dec) for pass details.\n");
    
    return 0;
}
