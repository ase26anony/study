/* test_auto_inc_dec.c
 * This program creates memory access patterns that should trigger
 * GCC's auto-increment/decrement optimization pass, specifically
 * the zero-offset pattern in find_inc() function.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 256
#define CHECK_VALUE 42

/* Global volatile to prevent dead code elimination */
volatile int g_result = 0;

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
     * Should generate: (mem (reg dst)) with offset 0 (after decrement)
     *                 (mem (reg src)) with offset 0 (after decrement)
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

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    /* Pattern: *p1++ = *--p2
     * Combines both increment and decrement patterns
     */
    int *p1 = arr;
    int *p2 = arr + n;
    while (p1 < p2) {
        *p1++ = *--p2;
    }
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char val, int n) {
    /* Pattern: *ptr++ = val
     * Simple byte-wise fill with post-increment
     */
    char *ptr = buf;
    while (n-- > 0) {
        *ptr++ = val;
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[SIZE];
    int dst[SIZE];
    int check_sum = 0;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i + CHECK_VALUE;
    }
    
    /* Test 1: Simple copy with post-increment */
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            return -1;
        }
    }
    
    /* Test 2: Sum calculation with post-increment */
    check_sum = sum_with_post_increment(src, SIZE);
    
    /* Test 3: Reverse with dual pointers */
    reverse_with_dual_pointers(dst, SIZE);
    
    /* Test 4: Fill buffer with post-increment */
    char buffer[SIZE];
    fill_with_post_increment(buffer, 'A', SIZE);
    
    /* Use results to prevent optimization */
    g_result = check_sum + buffer[0] + dst[0];
    
    return 0;
}

/* Additional test with different data types */
static void __attribute__((noinline))
test_char_pointers(void) {
    char src[SIZE * 2];
    char dst[SIZE * 2];
    
    /* Initialize */
    for (int i = 0; i < SIZE * 2; i++) {
        src[i] = (i % 26) + 'A';
    }
    
    /* Copy with post-increment - char version */
    char *d = dst;
    const char *s = src;
    int n = SIZE * 2;
    
    while (n-- > 0) {
        *d++ = *s++;
    }
    
    /* Verify */
    for (int i = 0; i < SIZE * 2; i++) {
        if (dst[i] != src[i]) {
            g_result = -1;
            return;
        }
    }
    
    g_result += dst[0];
}

/* Loop with conditional that still maintains pointer pattern */
static int __attribute__((noinline))
find_value_with_increment(const int *arr, int n, int target) {
    const int *ptr = arr;
    while (n-- > 0) {
        if (*ptr++ == target) {
            return 1;
        }
    }
    return 0;
}

int main(void) {
    int test_array[SIZE];
    
    /* Initialize test array */
    for (int i = 0; i < SIZE; i++) {
        test_array[i] = i * 2;
    }
    
    /* Run tests that should trigger auto-inc-dec patterns */
    if (test_combined_patterns() != 0) {
        printf("Test 1 failed\n");
        return 1;
    }
    
    test_char_pointers();
    
    /* Test search with increment */
    int found = find_value_with_increment(test_array, SIZE, 100);
    
    printf("All tests completed. g_result = %d, found = %d\n", 
           g_result, found);
    
    /* Additional verification */
    int sum = 0;
    int *ptr = test_array;
    for (int i = 0; i < SIZE; i++) {
        sum += *ptr++;
    }
    printf("Array sum: %d\n", sum);
    
    return 0;
}
