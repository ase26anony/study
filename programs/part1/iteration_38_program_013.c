/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-inc-dec pass logic
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

/* Prevent inlining to ensure loops remain intact for analysis */
static void __attribute__((noinline)) 
copy_forward_postinc(int *dst, const int *src, int n) {
    /* Pattern: post-increment with zero offset at memory access */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline)) 
copy_backward_predec(int *dst, const int *src, int n) {
    /* Move pointers to end for backward copy */
    dst += n;
    src += n;
    
    /* Pattern: pre-decrement with zero offset at memory access */
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline)) 
sum_array_postinc(const int *arr, int n) {
    /* Pattern: post-increment in read operation */
    int sum = 0;
    const int *p = arr;
    
    while (n-- > 0) {
        sum += *p++;  /* Memory read with zero offset */
    }
    return sum;
}

static void __attribute__((noinline))
fill_array_preinc(char *buf, char val, int n) {
    /* Pattern: pre-increment in write operation */
    char *p = buf + n;  /* Start at end */
    
    while (n-- > 0) {
        *--p = val;  /* Memory write with zero offset */
    }
}

/* More complex pattern: pointer arithmetic in loop */
static void __attribute__((noinline))
reverse_copy_postinc(int *dst, const int *src, int n) {
    const int *src_end = src + n;
    int *dst_start = dst;
    
    /* Copy in reverse using post-increment on both pointers */
    while (src_end > src) {
        *dst_start++ = *--src_end;  /* Mixed pre-decrement and post-increment */
    }
}

/* Simple memset-like pattern */
static void __attribute__((noinline))
set_memory_postinc(int *ptr, int value, int n) {
    while (n-- > 0) {
        *ptr++ = value;  /* Pure post-increment write */
    }
}

/* Test function with multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    char buffer[ARRAY_SIZE];
    int i, checksum = 0;
    
    /* Initialize source with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Test 1: Forward copy with post-increment */
    memset(dst1, 0, sizeof(dst1));
    copy_forward_postinc(dst1, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != src[i]) {
            printf("Error in forward copy at index %d\n", i);
            return -1;
        }
    }
    checksum += dst1[ARRAY_SIZE/2];
    
    /* Test 2: Backward copy with pre-decrement */
    memset(dst2, 0, sizeof(dst2));
    copy_backward_predec(dst2, src, ARRAY_SIZE);
    
    /* Verify backward copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst2[i] != src[i]) {
            printf("Error in backward copy at index %d\n", i);
            return -1;
        }
    }
    checksum += dst2[ARRAY_SIZE/4];
    
    /* Test 3: Sum with post-increment */
    int sum = sum_array_postinc(src, ARRAY_SIZE);
    checksum += sum % 1000;
    
    /* Test 4: Fill with pre-increment */
    memset(buffer, 0, sizeof(buffer));
    fill_array_preinc(buffer, 'A', ARRAY_SIZE/2);
    
    /* Verify fill */
    for (i = 0; i < ARRAY_SIZE/2; i++) {
        if (buffer[i] != 'A') {
            printf("Error in fill at index %d\n", i);
            return -1;
        }
    }
    checksum += buffer[0];
    
    /* Test 5: Reverse copy */
    int src_small[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int dst_rev[10];
    
    reverse_copy_postinc(dst_rev, src_small, 10);
    
    /* Verify reverse */
    for (i = 0; i < 10; i++) {
        if (dst_rev[i] != src_small[9 - i]) {
            printf("Error in reverse copy at index %d\n", i);
            return -1;
        }
    }
    checksum += dst_rev[5];
    
    /* Test 6: Set memory */
    int test_array[50];
    set_memory_postinc(test_array, 0x55AA55AA, 50);
    
    /* Verify set */
    for (i = 0; i < 50; i++) {
        if (test_array[i] != 0x55AA55AA) {
            printf("Error in set memory at index %d\n", i);
            return -1;
        }
    }
    checksum += test_array[25] & 0xFF;
    
    return checksum;  /* Return non-zero checksum to prevent dead code elimination */
}

/* Additional test with different data types */
static void __attribute__((noinline))
test_char_pointers(void) {
    char src[256];
    char dst[256];
    char *s = src;
    char *d = dst;
    int i;
    
    /* Initialize */
    for (i = 0; i < 256; i++) {
        src[i] = (char)(i & 0xFF);
    }
    
    /* Char copy with post-increment */
    for (i = 0; i < 256; i++) {
        *d++ = *s++;  /* Simple char copy pattern */
    }
    
    /* Verify */
    for (i = 0; i < 256; i++) {
        if (dst[i] != src[i]) {
            printf("Char copy failed at %d\n", i);
            break;
        }
    }
}

/* Main function to run all tests */
int main(void) {
    int result;
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Run combined tests */
    result = test_combined_patterns();
    if (result < 0) {
        printf("Test failed with error code: %d\n", result);
        return 1;
    }
    
    printf("Combined patterns test passed. Checksum: %d\n", result);
    
    /* Run char pointer test */
    test_char_pointers();
    printf("Char pointer test completed.\n");
    
    /* Additional simple direct test */
    {
        int a[100], b[100];
        int *pa = a, *pb = b;
        int i;
        
        for (i = 0; i < 100; i++) a[i] = i * 2;
        
        /* Very simple pattern that should trigger the pass */
        for (i = 0; i < 100; i++) {
            *pb++ = *pa++;  /* Direct pointer copy */
        }
        
        /* Quick verification */
        for (i = 0; i < 100; i++) {
            if (b[i] != a[i]) {
                printf("Direct copy failed at %d\n", i);
                return 1;
            }
        }
        printf("Direct copy test passed.\n");
    }
    
    printf("All tests completed successfully.\n");
    return 0;
}
