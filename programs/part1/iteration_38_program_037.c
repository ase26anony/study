/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 256
#define CHECK_VALUE 0xAA

/* Prevent inlining to ensure loops remain distinct */
static void __attribute__((noinline)) copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ 
     * Should generate: (mem (reg dst)) with offset 0, then increment */
    while (n-- > 0) {
        *dst++ = *src++;
    }
}

static void __attribute__((noinline)) reverse_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern: *--dst = *--src
     * Should generate: (mem (reg dst)) with offset 0, then decrement */
    dst += n;
    src += n;
    while (n-- > 0) {
        *--dst = *--src;
    }
}

static int __attribute__((noinline)) sum_with_post_increment(const int *arr, int n) {
    /* Pattern: sum += *ptr++ 
     * Multiple memory reads with zero offset */
    int sum = 0;
    const int *ptr = arr;
    while (n-- > 0) {
        sum += *ptr++;
    }
    return sum;
}

static void __attribute__((noinline)) fill_with_pre_increment(char *buf, int n, char val) {
    /* Pattern: *++ptr = val
     * Pre-increment version */
    char *ptr = buf - 1;
    while (n-- > 0) {
        *++ptr = val;
    }
}

static void __attribute__((noinline)) memset_pattern(void *dest, int pattern, int n) {
    /* Pattern: *p++ = pattern
     * Simple byte-by-byte fill */
    char *p = (char *)dest;
    while (n-- > 0) {
        *p++ = (char)pattern;
    }
}

/* Complex pattern: copy with overlapping regions using both inc and dec */
static void __attribute__((noinline)) move_memory(int *dst, const int *src, int n) {
    if (dst < src) {
        /* Forward copy with post-increment */
        while (n-- > 0) {
            *dst++ = *src++;
        }
    } else {
        /* Backward copy with pre-decrement */
        dst += n;
        src += n;
        while (n-- > 0) {
            *--dst = *--src;
        }
    }
}

int main(void) {
    int src[SIZE];
    int dst[SIZE];
    char buffer[SIZE];
    int i, result;
    
    /* Initialize source with pattern */
    for (i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test 1: Simple post-increment copy */
    memset(dst, 0, sizeof(dst));
    copy_with_post_increment(dst, src, SIZE);
    
    /* Verify copy */
    for (i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Post-increment copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Post-increment copy\n");
    
    /* Test 2: Pre-decrement reverse */
    memset(dst, 0, sizeof(dst));
    reverse_with_pre_decrement(dst, src, SIZE);
    
    /* Verify reverse */
    for (i = 0; i < SIZE; i++) {
        if (dst[i] != src[SIZE - 1 - i]) {
            printf("FAIL: Pre-decrement reverse mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Pre-decrement reverse\n");
    
    /* Test 3: Sum with post-increment */
    result = sum_with_post_increment(src, SIZE);
    int expected_sum = 0;
    for (i = 0; i < SIZE; i++) {
        expected_sum += src[i];
    }
    if (result != expected_sum) {
        printf("FAIL: Sum calculation mismatch: %d != %d\n", result, expected_sum);
        return 1;
    }
    printf("PASS: Sum with post-increment = %d\n", result);
    
    /* Test 4: Fill with pre-increment */
    memset(buffer, 0, sizeof(buffer));
    fill_with_pre_increment(buffer, SIZE, CHECK_VALUE);
    
    for (i = 0; i < SIZE; i++) {
        if (buffer[i] != CHECK_VALUE) {
            printf("FAIL: Pre-increment fill mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Pre-increment fill\n");
    
    /* Test 5: Byte fill with post-increment */
    memset(buffer, 0, sizeof(buffer));
    memset_pattern(buffer, CHECK_VALUE, SIZE);
    
    for (i = 0; i < SIZE; i++) {
        if (buffer[i] != CHECK_VALUE) {
            printf("FAIL: memset pattern mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Byte fill with post-increment\n");
    
    /* Test 6: Overlapping memory move */
    memmove(dst, src, sizeof(src));
    move_memory(dst + 10, dst, SIZE - 10);  /* Overlapping forward move */
    
    for (i = 0; i < SIZE - 10; i++) {
        if (dst[i + 10] != src[i]) {
            printf("FAIL: Overlapping move mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Overlapping memory move\n");
    
    printf("\nAll tests passed! The auto-inc-dec pass should have optimized these patterns.\n");
    
    /* Additional test with different data types to explore more patterns */
    {
        short short_src[SIZE];
        short short_dst[SIZE];
        long long ll_src[SIZE];
        long long ll_dst[SIZE];
        
        for (i = 0; i < SIZE; i++) {
            short_src[i] = (short)(i * 5);
            ll_src[i] = (long long)i * 1000;
        }
        
        /* Short type post-increment */
        short *sp = short_dst;
        const short *ss = short_src;
        for (i = 0; i < SIZE; i++) {
            *sp++ = *ss++;
        }
        
        /* Long long type pre-decrement */
        long long *lp = ll_dst + SIZE;
        const long long *ls = ll_src + SIZE;
        for (i = 0; i < SIZE; i++) {
            *--lp = *--ls;
        }
        
        printf("Additional type patterns completed.\n");
    }
    
    return 0;
}
