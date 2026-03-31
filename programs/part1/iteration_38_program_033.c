/* test_auto_inc_dec.c
 * Program designed to trigger zero-offset memory access patterns
 * for GCC's auto-inc-dec pass (lines 1352-1358 in auto-inc-dec.cc)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 1024
#define CHECK_VALUE 42

/* Prevent inlining to ensure loops remain intact for analysis */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n--) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates zero-offset accesses */
    while (n--) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates zero-offset read */
    while (n--) {
        sum += *p++;  /* Memory read with zero offset */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: *start++ = *end-- creates zero-offset accesses */
    while (start < end) {
        int temp = *end;    /* Read from end pointer */
        *end-- = *start;    /* Write to end with post-decrement */
        *start++ = temp;    /* Write to start with post-increment */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char value, int n) {
    char *p = buf;
    
    /* Simple pattern: *p++ = value */
    while (n--) {
        *p++ = value;  /* Write with post-increment */
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[SIZE];
    int dst[SIZE];
    int checksum = 0;
    
    /* Initialize source array */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i + CHECK_VALUE;
    }
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            return -1;
        }
    }
    
    /* Test 2: Pre-decrement copy (reverse copy) */
    int src2[SIZE];
    for (int i = 0; i < SIZE; i++) {
        src2[i] = SIZE - i;
    }
    copy_with_pre_decrement(dst, src2, SIZE);
    
    /* Test 3: Sum with post-increment */
    checksum = sum_with_post_increment(src, SIZE);
    
    /* Test 4: Reverse array using dual pointers */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    reverse_with_dual_pointers(arr, SIZE);
    
    /* Verify reversal */
    for (int i = 0; i < SIZE; i++) {
        if (arr[i] != SIZE - 1 - i) {
            return -2;
        }
    }
    
    /* Test 5: Character buffer fill */
    char buffer[SIZE];
    fill_with_post_increment(buffer, 'A', SIZE);
    
    for (int i = 0; i < SIZE; i++) {
        if (buffer[i] != 'A') {
            return -3;
        }
    }
    
    return checksum;  /* Return non-zero to prevent dead code elimination */
}

/* Main function that drives the tests */
int main(void) {
    int result = test_combined_patterns();
    
    if (result < 0) {
        printf("Test failed with error code: %d\n", result);
        return 1;
    }
    
    printf("Test passed! Checksum: %d\n", result);
    printf("Patterns should trigger auto-inc-dec pass zero-offset logic.\n");
    
    /* Additional simple direct test */
    {
        int a[10], b[10];
        int *src = a;
        int *dst = b;
        
        /* Simple loop that should generate the pattern */
        for (int i = 0; i < 10; i++) {
            *dst++ = *src++;  /* Core pattern for zero-offset */
        }
    }
    
    return 0;
}
