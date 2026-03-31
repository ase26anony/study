/* test_auto_inc_dec.c
 * Program to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define CHECK_VALUE 42

/* Global volatile to prevent dead code elimination */
volatile int global_check = 0;

/* Function 1: Post-increment pattern - should trigger auto-increment */
static void __attribute__((noinline)) copy_with_post_inc(int *dst, const int *src, int n)
{
    /* Simple loop with post-increment - generates base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;  /* Memory access with zero offset, then increment */
    }
}

/* Function 2: Pre-decrement pattern - should trigger auto-decrement */
static void __attribute__((noinline)) reverse_with_pre_dec(int *dst, const int *src, int n)
{
    /* Start from end and work backwards with pre-decrement */
    dst += n - 1;
    src += n - 1;
    
    while (n-- > 0) {
        *dst-- = *src--;  /* Memory access with zero offset, then decrement */
    }
}

/* Function 3: Mixed operations to create more complex pattern */
static int __attribute__((noinline)) sum_with_inc(const int *arr, int n)
{
    int sum = 0;
    const int *p = arr;
    
    while (n-- > 0) {
        sum += *p++;  /* Read with post-increment */
    }
    
    return sum;
}

/* Function 4: Array initialization with pointer arithmetic */
static void __attribute__((noinline)) init_with_inc(int *arr, int n, int value)
{
    int *p = arr;
    
    while (n-- > 0) {
        *p++ = value;  /* Write with post-increment */
    }
}

/* Function 5: Memory copy with char pointers (byte access) */
static void __attribute__((noinline)) memcpy_with_inc(char *dst, const char *src, int n)
{
    while (n-- > 0) {
        *dst++ = *src++;  /* Byte access with post-increment */
    }
}

/* Function 6: Search pattern with pointer */
static int __attribute__((noinline)) find_with_inc(const int *arr, int n, int target)
{
    const int *p = arr;
    int count = 0;
    
    while (n-- > 0) {
        if (*p++ == target) {  /* Read and compare with post-increment */
            count++;
        }
    }
    
    return count;
}

int main(void)
{
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    char src_char[ARRAY_SIZE];
    char dst_char[ARRAY_SIZE];
    int i, result;
    
    /* Initialize source arrays */
    for (i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i % 100;
        src_char[i] = (char)(i % 256);
    }
    
    printf("Testing auto-inc-dec optimization patterns...\n");
    
    /* Test 1: Post-increment copy */
    memset(dst1, 0, sizeof(dst1));
    copy_with_post_inc(dst1, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != src[i]) {
            printf("FAIL: Post-increment copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Post-increment copy\n");
    
    /* Test 2: Pre-decrement reverse */
    memset(dst2, 0, sizeof(dst2));
    reverse_with_pre_dec(dst2, src, ARRAY_SIZE);
    
    /* Verify reverse */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst2[i] != src[ARRAY_SIZE - 1 - i]) {
            printf("FAIL: Pre-decrement reverse mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Pre-decrement reverse\n");
    
    /* Test 3: Sum calculation */
    result = sum_with_inc(src, ARRAY_SIZE);
    printf("Sum result: %d\n", result);
    
    /* Test 4: Array initialization */
    init_with_inc(dst1, ARRAY_SIZE, CHECK_VALUE);
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != CHECK_VALUE) {
            printf("FAIL: Init with increment mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Init with increment\n");
    
    /* Test 5: Char array copy */
    memset(dst_char, 0, sizeof(dst_char));
    memcpy_with_inc(dst_char, src_char, ARRAY_SIZE);
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst_char[i] != src_char[i]) {
            printf("FAIL: Char copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Char copy with increment\n");
    
    /* Test 6: Search pattern */
    result = find_with_inc(src, ARRAY_SIZE, CHECK_VALUE);
    printf("Found %d occurrences of %d\n", result, CHECK_VALUE);
    
    /* Store result in global to prevent optimization */
    global_check = result;
    
    printf("All tests passed! Auto-inc-dec patterns should be triggered.\n");
    
    return 0;
}
