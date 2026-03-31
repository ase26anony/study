/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 256
#define CHECK_VALUE 0xAA

/* Prevent inlining to ensure loops remain intact for analysis */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ 
     * Should generate: (mem (reg dst)) with offset 0 */
    while (n-- > 0) {
        *dst++ = *src++;
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src
     * Should generate: (mem (reg dst)) with offset 0 */
    dst += n;
    src += n;
    while (n-- > 0) {
        *--dst = *--src;
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    /* Pattern: sum += *ptr++ 
     * Should generate: (mem (reg ptr)) with offset 0 */
    int sum = 0;
    const int *ptr = arr;
    while (n-- > 0) {
        sum += *ptr++;
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(char *dst, const char *src, int n) {
    /* Pattern: *dst++ = *src-- 
     * Mixed increment/decrement pattern */
    const char *src_end = src + n - 1;
    while (n-- > 0) {
        *dst++ = *src_end--;
    }
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char val, int n) {
    /* Pattern: *ptr++ = val
     * Simple write pattern */
    char *ptr = buf;
    while (n-- > 0) {
        *ptr++ = val;
    }
}

/* Test functions that should trigger the specific uncovered code path */
static int __attribute__((noinline))
test_post_inc_pattern(void) {
    int src[SIZE];
    int dst[SIZE];
    
    /* Initialize source with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Clear destination */
    memset(dst, 0, sizeof(dst));
    
    /* This should generate the target RTL pattern */
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            return 0;
        }
    }
    return 1;
}

static int __attribute__((noinline))
test_pre_dec_pattern(void) {
    int src[SIZE];
    int dst[SIZE];
    
    /* Initialize source with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 5 + 2;
    }
    
    /* Clear destination */
    memset(dst, 0, sizeof(dst));
    
    /* This should generate the target RTL pattern */
    copy_with_pre_decrement(dst, src, SIZE);
    
    /* Verify copy (reversed) */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            return 0;
        }
    }
    return 1;
}

static int __attribute__((noinline))
test_mixed_patterns(void) {
    char src[SIZE];
    char dst[SIZE];
    
    /* Initialize source */
    for (int i = 0; i < SIZE; i++) {
        src[i] = (char)(i & 0xFF);
    }
    
    /* Test reverse copy */
    reverse_with_dual_pointers(dst, src, SIZE);
    
    /* Verify reverse */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[SIZE - 1 - i]) {
            return 0;
        }
    }
    
    /* Test fill pattern */
    fill_with_post_increment(dst, CHECK_VALUE, SIZE);
    
    /* Verify fill */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != CHECK_VALUE) {
            return 0;
        }
    }
    
    return 1;
}

int main(void) {
    int success = 1;
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test 1: Post-increment pattern */
    if (!test_post_inc_pattern()) {
        printf("FAIL: Post-increment pattern test\n");
        success = 0;
    } else {
        printf("PASS: Post-increment pattern test\n");
    }
    
    /* Test 2: Pre-decrement pattern */
    if (!test_pre_dec_pattern()) {
        printf("FAIL: Pre-decrement pattern test\n");
        success = 0;
    } else {
        printf("PASS: Pre-decrement pattern test\n");
    }
    
    /* Test 3: Mixed patterns */
    if (!test_mixed_patterns()) {
        printf("FAIL: Mixed patterns test\n");
        success = 0;
    } else {
        printf("PASS: Mixed patterns test\n");
    }
    
    /* Additional test: Sum calculation with post-increment */
    {
        int arr[SIZE];
        int expected_sum = 0;
        
        for (int i = 0; i < SIZE; i++) {
            arr[i] = i + 1;
            expected_sum += i + 1;
        }
        
        int calculated_sum = sum_with_post_increment(arr, SIZE);
        
        if (calculated_sum != expected_sum) {
            printf("FAIL: Sum calculation test (got %d, expected %d)\n", 
                   calculated_sum, expected_sum);
            success = 0;
        } else {
            printf("PASS: Sum calculation test\n");
        }
    }
    
    if (success) {
        printf("\nAll tests passed! The auto-inc-dec pass should have processed the patterns.\n");
        printf("Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c test_auto_inc_dec.c\n");
        printf("Check the generated .auto-inc-dec file for pass activity.\n");
    } else {
        printf("\nSome tests failed.\n");
    }
    
    return success ? 0 : 1;
}
