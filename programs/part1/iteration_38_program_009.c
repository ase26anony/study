/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 256

/* Prevent inlining to ensure loops remain distinct */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ - should generate base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;  /* Zero offset at memory access time */
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Adjust pointers to end of arrays for pre-decrement */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src - should generate base+0 addressing */
    while (n-- > 0) {
        *--dst = *--src;  /* Zero offset at memory access time */
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ - zero offset memory read */
    while (n-- > 0) {
        sum += *p++;  /* Base register + 0 offset */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: *start++ = *end-- - both have zero offset */
    while (start < end) {
        int temp = *end;      /* Zero offset read */
        *end-- = *start;      /* Zero offset write then decrement */
        *start++ = temp;      /* Zero offset write then increment */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char value, int n) {
    /* Pattern: *buf++ = value - simple zero-offset store */
    while (n-- > 0) {
        *buf++ = value;  /* Base + 0 offset store */
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    int result = 0;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Test 1: Post-increment copy */
    memset(dst1, 0, sizeof(dst1));
    copy_with_post_increment(dst1, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != src[i]) {
            result |= 1;
        }
    }
    
    /* Test 2: Pre-decrement copy (reverse copy) */
    memset(dst2, 0, sizeof(dst2));
    copy_with_pre_decrement(dst2, src, ARRAY_SIZE);
    
    /* Verify reverse copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst2[i] != src[ARRAY_SIZE - 1 - i]) {
            result |= 2;
        }
    }
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    if (sum != expected_sum) {
        result |= 4;
    }
    
    /* Test 4: Array reversal */
    int test_arr[10];
    for (int i = 0; i < 10; i++) {
        test_arr[i] = i;
    }
    reverse_with_dual_pointers(test_arr, 10);
    for (int i = 0; i < 10; i++) {
        if (test_arr[i] != 9 - i) {
            result |= 8;
        }
    }
    
    /* Test 5: Char buffer fill */
    char buffer[100];
    fill_with_post_increment(buffer, 'A', 100);
    for (int i = 0; i < 100; i++) {
        if (buffer[i] != 'A') {
            result |= 16;
        }
    }
    
    return result;
}

int main(void) {
    int result = test_combined_patterns();
    
    if (result == 0) {
        printf("All tests passed - patterns executed correctly\n");
        printf("Check RTL dumps for auto-inc-dec pass activity\n");
    } else {
        printf("Test failed with error code: %d\n", result);
    }
    
    /* Additional simple direct test for clarity */
    {
        int a[5] = {1, 2, 3, 4, 5};
        int b[5] = {0};
        int *src_ptr = a;
        int *dst_ptr = b;
        
        /* Very clear post-increment pattern */
        for (int i = 0; i < 5; i++) {
            *dst_ptr++ = *src_ptr++;  /* Should trigger zero-offset pattern */
        }
        
        printf("Simple copy test: ");
        int ok = 1;
        for (int i = 0; i < 5; i++) {
            if (a[i] != b[i]) ok = 0;
        }
        printf("%s\n", ok ? "PASS" : "FAIL");
    }
    
    return result;
}
