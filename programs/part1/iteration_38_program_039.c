/* test_auto_inc_dec.c
 * Program to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>

#define SIZE 256

/* Global volatile to prevent dead code elimination */
volatile int checksum = 0;

/* Noinline to ensure function boundaries are preserved */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for reverse copy */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates base+0 addressing */
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates base+0 addressing */
    while (n-- > 0) {
        sum += *p++;  /* Should generate (mem (reg)) with offset 0 */
    }
    return sum;
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char val, int n) {
    char *p = buf;
    
    /* Pattern: *p++ = val creates base+0 addressing */
    while (n-- > 0) {
        *p++ = val;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
reverse_with_dual_pointers(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src + n - 1;
    
    /* Pattern: *d++ = *s-- creates base+0 addressing on both sides */
    while (n-- > 0) {
        *d++ = *s--;  /* Both memory accesses should have offset 0 */
    }
}

/* Test function with multiple patterns */
static void __attribute__((noinline))
test_multiple_patterns(void) {
    int src[SIZE], dst[SIZE];
    char buf1[SIZE], buf2[SIZE];
    
    /* Initialize source arrays */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
        buf1[i] = (char)(i & 0xFF);
    }
    
    /* Test 1: Simple post-increment copy */
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("Error in copy_with_post_increment at index %d\n", i);
            return;
        }
    }
    
    /* Test 2: Pre-decrement reverse copy */
    int src2[SIZE], dst2[SIZE];
    for (int i = 0; i < SIZE; i++) {
        src2[i] = i * 2;
    }
    copy_with_pre_decrement(dst2, src2, SIZE);
    
    /* Verify reverse copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst2[i] != src2[SIZE - 1 - i]) {
            printf("Error in copy_with_pre_decrement at index %d\n", i);
            return;
        }
    }
    
    /* Test 3: Sum calculation with post-increment */
    int sum = sum_with_post_increment(src, SIZE);
    int expected_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        expected_sum += src[i];
    }
    if (sum != expected_sum) {
        printf("Error in sum_with_post_increment: %d != %d\n", sum, expected_sum);
        return;
    }
    
    /* Test 4: Fill buffer with post-increment */
    fill_with_post_increment(buf2, 'A', SIZE);
    for (int i = 0; i < SIZE; i++) {
        if (buf2[i] != 'A') {
            printf("Error in fill_with_post_increment at index %d\n", i);
            return;
        }
    }
    
    /* Test 5: Reverse copy with dual pointers */
    char buf3[SIZE];
    reverse_with_dual_pointers(buf3, buf1, SIZE);
    for (int i = 0; i < SIZE; i++) {
        if (buf3[i] != buf1[SIZE - 1 - i]) {
            printf("Error in reverse_with_dual_pointers at index %d\n", i);
            return;
        }
    }
    
    /* Store checksum to volatile to prevent optimization */
    checksum = sum + buf3[0];
    
    printf("All tests passed! Auto-inc-dec patterns should be triggered.\n");
}

/* Main function with different loop structures */
int main(void) {
    /* Test with different sizes to trigger various optimizations */
    test_multiple_patterns();
    
    /* Additional simple test cases */
    {
        int small_src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        int small_dst[10];
        
        /* Simple forward copy - most likely to trigger the pattern */
        int *d = small_dst;
        const int *s = small_src;
        for (int i = 0; i < 10; i++) {
            *d++ = *s++;  /* Base+0 addressing pattern */
        }
        
        /* Verify */
        for (int i = 0; i < 10; i++) {
            if (small_dst[i] != small_src[i]) {
                printf("Small copy failed at index %d\n", i);
                return 1;
            }
        }
    }
    
    /* Test with char pointers (different size might affect pattern) */
    {
        char message[] = "Hello, World!";
        char buffer[20];
        char *p = buffer;
        const char *q = message;
        
        /* Copy string with post-increment */
        while (*q) {
            *p++ = *q++;  /* Base+0 addressing pattern */
        }
        *p = '\0';
        
        if (strcmp(buffer, message) != 0) {
            printf("String copy failed\n");
            return 1;
        }
    }
    
    printf("All verification tests passed!\n");
    return 0;
}
