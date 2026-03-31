/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

/* Prevent inlining to ensure loops remain distinct */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates zero-offset accesses */
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates zero-offset read */
    while (n-- > 0) {
        sum += *p++;  /* Memory access with pointer as base, offset 0 */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src + n - 1;
    
    /* Pattern: *d++ = *s-- uses both post-increment and post-decrement */
    while (n-- > 0) {
        *d++ = *s--;  /* Both accesses should have zero offset */
    }
}

static void __attribute__((noinline))
fill_with_pre_increment(int *arr, int value, int n) {
    int *p = arr + n;
    
    /* Pattern: *--p = value creates zero-offset write with pre-decrement */
    while (n-- > 0) {
        *--p = value + n;  /* Memory write with pre-decrement */
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    char src_str[ARRAY_SIZE];
    char dst_str[ARRAY_SIZE];
    int i, checksum = 0;
    
    /* Initialize source arrays with known values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
        src_str[i] = 'A' + (i % 26);
    }
    
    /* Test 1: Post-increment copy */
    memset(dst1, 0, sizeof(dst1));
    copy_with_post_increment(dst1, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != src[i]) {
            printf("Error: Post-increment copy failed at index %d\n", i);
            return -1;
        }
    }
    checksum += dst1[ARRAY_SIZE/2];
    
    /* Test 2: Pre-decrement copy */
    memset(dst2, 0, sizeof(dst2));
    copy_with_pre_decrement(dst2, src, ARRAY_SIZE);
    
    /* Verify reverse copy (should be same as forward for this test) */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst2[i] != src[i]) {
            printf("Error: Pre-decrement copy failed at index %d\n", i);
            return -1;
        }
    }
    checksum += dst2[ARRAY_SIZE/2];
    
    /* Test 3: Sum with post-increment */
    int calculated_sum = sum_with_post_increment(src, ARRAY_SIZE);
    
    /* Calculate expected sum */
    int expected_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (calculated_sum != expected_sum) {
        printf("Error: Sum calculation failed: got %d, expected %d\n", 
               calculated_sum, expected_sum);
        return -1;
    }
    checksum += calculated_sum % 1000;
    
    /* Test 4: Reverse string with dual pointers */
    memset(dst_str, 0, sizeof(dst_str));
    reverse_with_dual_pointers(dst_str, src_str, ARRAY_SIZE);
    
    /* Verify reversal */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst_str[i] != src_str[ARRAY_SIZE - 1 - i]) {
            printf("Error: Reverse copy failed at index %d\n", i);
            return -1;
        }
    }
    checksum += dst_str[ARRAY_SIZE/2];
    
    /* Test 5: Fill with pre-increment */
    int test_arr[100];
    fill_with_pre_increment(test_arr, 42, 100);
    
    /* Verify fill */
    for (i = 0; i < 100; i++) {
        if (test_arr[i] != 42 + (99 - i)) {
            printf("Error: Pre-increment fill failed at index %d\n", i);
            return -1;
        }
    }
    checksum += test_arr[50];
    
    return checksum; /* Return non-zero checksum to prevent dead code elimination */
}

int main(void) {
    int result = test_combined_patterns();
    
    if (result >= 0) {
        printf("All auto-inc-dec patterns executed successfully. Checksum: %d\n", result);
        return 0;
    } else {
        printf("Test failed\n");
        return 1;
    }
}
