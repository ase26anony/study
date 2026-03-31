/* test_auto_inc_dec.c
 * This program creates memory access patterns that should trigger
 * the auto-increment/decrement optimization in GCC's RTL backend.
 * The goal is to reach the uncovered code in find_inc() where
 * reg1_val = 0 for zero-offset memory accesses.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

/* Use noinline to prevent inlining and preserve loop structure */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates base+0 offset memory accesses */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src creates base+0 offset memory accesses */
    dst += n;
    src += n;
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    /* Pattern: sum += *ptr++ creates read with base+0 offset */
    int sum = 0;
    const int *ptr = arr;
    while (n-- > 0) {
        sum += *ptr++;  /* Read with zero offset */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_array(int *arr, int n) {
    /* Mix of increment and decrement patterns */
    int *start = arr;
    int *end = arr + n - 1;
    while (start < end) {
        int temp = *start;
        *start++ = *end;    /* Write with post-increment */
        *end-- = temp;      /* Write with post-decrement */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(int *dst, int value, int n) {
    /* Simple write pattern: *dst++ = value */
    while (n-- > 0) {
        *dst++ = value;  /* Write with zero offset */
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    int i, result = 0;
    
    /* Initialize source array with predictable values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Test 1: Simple copy with post-increment */
    memset(dst1, 0, sizeof(dst1));
    copy_with_post_increment(dst1, src, ARRAY_SIZE);
    
    /* Verify the copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != src[i]) {
            result |= 1;
        }
    }
    
    /* Test 2: Copy with pre-decrement (reverse copy) */
    memset(dst2, 0, sizeof(dst2));
    copy_with_pre_decrement(dst2 + ARRAY_SIZE - 1, src + ARRAY_SIZE - 1, ARRAY_SIZE);
    
    /* Verify the reverse copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst2[i] != src[i]) {
            result |= 2;
        }
    }
    
    /* Test 3: Sum calculation with post-increment */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    int expected_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    if (sum != expected_sum) {
        result |= 4;
    }
    
    /* Test 4: Array reversal */
    int rev[ARRAY_SIZE];
    memcpy(rev, src, sizeof(rev));
    reverse_array(rev, ARRAY_SIZE);
    
    /* Verify reversal */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (rev[i] != src[ARRAY_SIZE - 1 - i]) {
            result |= 8;
        }
    }
    
    /* Test 5: Fill with post-increment */
    int fill_value = 0xABCD;
    fill_with_post_increment(dst1, fill_value, ARRAY_SIZE);
    
    /* Verify fill */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != fill_value) {
            result |= 16;
        }
    }
    
    return result;  /* 0 means all tests passed */
}

/* Additional test with char pointers (different data type) */
static void __attribute__((noinline))
copy_chars_with_inc(char *dst, const char *src, int n) {
    while (n-- > 0) {
        *dst++ = *src++;  /* Char version of the pattern */
    }
}

/* Test with local pointer variables in main loop */
static int __attribute__((noinline))
sum_with_local_pointers(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    const int *end = arr + n;
    
    /* Clear loop with local pointer variable */
    while (p < end) {
        sum += *p;  /* Read with zero offset */
        p++;        /* Increment after use */
    }
    return sum;
}

int main(void) {
    int test_result;
    
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Run the main test */
    test_result = test_combined_patterns();
    
    if (test_result == 0) {
        printf("All tests passed successfully.\n");
    } else {
        printf("Test failed with error code: %d\n", test_result);
    }
    
    /* Additional test with char pointers */
    char src_str[] = "Test string for char pointer increment";
    char dst_str[sizeof(src_str)];
    
    copy_chars_with_inc(dst_str, src_str, sizeof(src_str));
    if (memcmp(src_str, dst_str, sizeof(src_str)) != 0) {
        printf("Char copy test failed.\n");
        test_result |= 32;
    }
    
    /* Test with alternative pointer usage pattern */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    int sum1 = sum_with_post_increment(array, 100);
    int sum2 = sum_with_local_pointers(array, 100);
    
    if (sum1 != sum2) {
        printf("Sum calculation mismatch.\n");
        test_result |= 64;
    }
    
    printf("Test completed with result code: %d\n", test_result);
    printf("(0 means all tests passed)\n");
    
    return test_result;
}
