/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass with zero-offset memory access
 * patterns, specifically targeting lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <string.h>

/* Global volatile to prevent dead code elimination */
volatile int g_checksum = 0;

/* Noinline to ensure function boundaries are preserved */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) 
{
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n)
{
    /* Move pointers to end for reverse copy */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates zero-offset accesses */
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n)
{
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates zero-offset read */
    while (n-- > 0) {
        sum += *p++;  /* Memory read with zero offset */
    }
    
    return sum;
}

static void __attribute__((noinline))
fill_with_post_increment(int *arr, int value, int n)
{
    int *p = arr;
    
    /* Pattern: *p++ = value creates zero-offset write */
    while (n-- > 0) {
        *p++ = value;  /* Memory write with zero offset */
    }
}

static void __attribute__((noinline))
reverse_with_both_modes(int *arr, int n)
{
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Mixed pre-decrement and post-increment */
    while (start < end) {
        int temp = *start;
        *start++ = *end;    /* Post-increment write */
        *end-- = temp;      /* Pre-decrement write */
    }
}

/* Test function that combines multiple patterns */
static void __attribute__((noinline))
test_combined_patterns(void)
{
    const int SIZE = 256;
    int src[SIZE];
    int dst[SIZE];
    int i;
    
    /* Initialize source array */
    for (i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Test 1: Simple post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("Error in copy_with_post_increment at index %d\n", i);
            return;
        }
    }
    
    /* Test 2: Pre-decrement reverse copy */
    memset(dst, 0, sizeof(dst));
    copy_with_pre_decrement(dst, src, SIZE);
    
    /* Verify reverse copy */
    for (i = 0; i < SIZE; i++) {
        if (dst[i] != src[SIZE - 1 - i]) {
            printf("Error in copy_with_pre_decrement at index %d\n", i);
            return;
        }
    }
    
    /* Test 3: Sum calculation with post-increment */
    int sum = sum_with_post_increment(src, SIZE);
    int expected_sum = 0;
    for (i = 0; i < SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (sum != expected_sum) {
        printf("Error in sum_with_post_increment: %d != %d\n", 
               sum, expected_sum);
        return;
    }
    
    /* Test 4: Fill with post-increment */
    fill_with_post_increment(dst, 42, SIZE);
    for (i = 0; i < SIZE; i++) {
        if (dst[i] != 42) {
            printf("Error in fill_with_post_increment at index %d\n", i);
            return;
        }
    }
    
    /* Test 5: Reverse with mixed modes */
    for (i = 0; i < SIZE; i++) {
        dst[i] = i;
    }
    reverse_with_both_modes(dst, SIZE);
    for (i = 0; i < SIZE; i++) {
        if (dst[i] != SIZE - 1 - i) {
            printf("Error in reverse_with_both_modes at index %d\n", i);
            return;
        }
    }
    
    /* Store checksum to volatile global to prevent optimization */
    g_checksum = sum;
    
    printf("All tests passed! Auto-inc-dec patterns successfully generated.\n");
}

/* Additional test with char pointers (different width) */
static void __attribute__((noinline))
test_char_pointers(void)
{
    const int SIZE = 512;
    char src[SIZE];
    char dst[SIZE];
    char *s = src;
    char *d = dst;
    int i;
    
    /* Initialize */
    for (i = 0; i < SIZE; i++) {
        src[i] = (i % 256);
    }
    
    /* Char copy with post-increment */
    for (i = 0; i < SIZE; i++) {
        *d++ = *s++;  /* Zero-offset char accesses */
    }
    
    /* Verify */
    for (i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("Error in char pointer test at index %d\n", i);
            return;
        }
    }
    
    printf("Char pointer test passed!\n");
}

/* Main driver */
int main(void)
{
    printf("Testing auto-inc-dec optimization patterns...\n");
    
    test_combined_patterns();
    test_char_pointers();
    
    /* Use the checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", g_checksum);
    
    return 0;
}
