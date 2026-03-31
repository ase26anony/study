/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int g_checksum = 0;

/* Function attributes to prevent inlining and preserve loop structure */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ generates base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src generates base+0 addressing */
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *src, int n) {
    int sum = 0;
    const int *p = src;
    
    /* Pattern: sum += *p++ generates base+0 addressing */
    while (n-- > 0) {
        sum += *p++;  /* Should generate (mem (reg)) with offset 0 */
    }
    
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src + n - 1;
    
    /* Pattern: *d++ = *s-- generates base+0 addressing for both */
    while (n-- > 0) {
        *d++ = *s--;  /* Both memory accesses should be base+0 */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(short *arr, short value, int n) {
    short *p = arr;
    
    /* Pattern: *p++ = value generates base+0 addressing */
    while (n-- > 0) {
        *p++ = value;  /* Should generate (mem (reg)) with offset 0 */
    }
}

/* Test function that combines multiple patterns */
static void __attribute__((noinline))
test_combined_patterns(void) {
    const int SIZE = 256;
    int src[SIZE];
    int dst[SIZE];
    char src_str[SIZE];
    char dst_str[SIZE];
    short short_arr[SIZE];
    
    /* Initialize source data */
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 3;
        src_str[i] = 'A' + (i % 26);
        short_arr[i] = (short)(i * 2);
    }
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("Error: Post-increment copy failed at index %d\n", i);
            return;
        }
    }
    
    /* Test 2: Pre-decrement copy */
    memset(dst, 0, sizeof(dst));
    copy_with_pre_decrement(dst, src, SIZE);
    
    /* Verify copy */
    for (int i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("Error: Pre-decrement copy failed at index %d\n", i);
            return;
        }
    }
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, SIZE);
    int expected_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (sum != expected_sum) {
        printf("Error: Sum calculation failed: got %d, expected %d\n", 
               sum, expected_sum);
        return;
    }
    
    /* Test 4: Reverse with dual pointers */
    reverse_with_dual_pointers(dst_str, src_str, SIZE);
    
    /* Verify reverse */
    for (int i = 0; i < SIZE; i++) {
        if (dst_str[i] != src_str[SIZE - 1 - i]) {
            printf("Error: Reverse failed at index %d\n", i);
            return;
        }
    }
    
    /* Test 5: Fill with post-increment */
    fill_with_post_increment(short_arr, 0x7FFF, SIZE / 2);
    
    /* Verify fill */
    for (int i = 0; i < SIZE / 2; i++) {
        if (short_arr[i] != 0x7FFF) {
            printf("Error: Fill failed at index %d\n", i);
            return;
        }
    }
    
    /* Store checksum to volatile global to prevent optimization */
    g_checksum = sum;
    
    printf("All tests passed successfully!\n");
}

/* Main function with multiple test cases */
int main(void) {
    printf("Testing auto-inc-dec pass patterns...\n");
    
    /* Run the combined test */
    test_combined_patterns();
    
    /* Additional simple test cases to increase coverage */
    {
        int small_src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        int small_dst[10];
        
        /* Simple post-increment */
        int *p = small_src;
        int *q = small_dst;
        for (int i = 0; i < 10; i++) {
            *q++ = *p++;  /* Base+0 pattern */
        }
        
        /* Simple pre-decrement */
        p = small_src + 9;
        q = small_dst + 9;
        for (int i = 0; i < 10; i++) {
            *q-- = *p--;  /* Base+0 pattern */
        }
    }
    
    /* Array initialization with pointer */
    {
        char buffer[100];
        char *ptr = buffer;
        int count = sizeof(buffer);
        
        while (count-- > 0) {
            *ptr++ = '.';  /* Base+0 pattern */
        }
    }
    
    return 0;
}
