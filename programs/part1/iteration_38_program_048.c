/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Global volatile to prevent dead code elimination */
volatile int g_checksum = 0;

/* Noinline to ensure function boundaries are preserved */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for reverse copy */
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
        sum += *p++;  /* Memory read with zero offset */
    }
    return sum;
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char value, int n) {
    char *p = buf;
    
    /* Pattern: *p++ = value creates zero-offset write */
    while (n-- > 0) {
        *p++ = value;  /* Memory write with zero offset */
    }
}

static void __attribute__((noinline))
reverse_with_both_modes(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src + n - 1;
    
    /* Mixed pattern: *d++ = *s-- */
    while (n-- > 0) {
        *d++ = *s--;  /* Both memory accesses should have zero offset */
    }
}

/* Test different data types and access patterns */
static void test_int_patterns(void) {
    const int N = 256;
    int src[N], dst1[N], dst2[N];
    int i, sum_expected = 0;
    
    /* Initialize source with pattern */
    for (i = 0; i < N; i++) {
        src[i] = i * 3 + 1;
        sum_expected += src[i];
    }
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst1, src, N);
    
    /* Test 2: Pre-decrement reverse copy */
    copy_with_pre_decrement(dst2, src, N);
    
    /* Test 3: Sum calculation with post-increment */
    int sum = sum_with_post_increment(src, N);
    
    /* Verification */
    int ok1 = 1, ok2 = 1;
    for (i = 0; i < N; i++) {
        if (dst1[i] != src[i]) ok1 = 0;
        if (dst2[N-1-i] != src[i]) ok2 = 0;
    }
    
    g_checksum = sum;
    printf("Test 1 (post-inc copy): %s\n", ok1 ? "PASS" : "FAIL");
    printf("Test 2 (pre-dec copy): %s\n", ok2 ? "PASS" : "FAIL");
    printf("Test 3 (sum calculation): %s (sum=%d, expected=%d)\n", 
           sum == sum_expected ? "PASS" : "FAIL", sum, sum_expected);
}

static void test_char_patterns(void) {
    const int N = 128;
    char src[N], dst1[N], dst2[N];
    int i;
    
    /* Initialize source */
    for (i = 0; i < N; i++) {
        src[i] = (char)(i % 26 + 'A');
    }
    
    /* Test 4: Fill with post-increment */
    fill_with_post_increment(dst1, 'X', N);
    
    /* Test 5: Reverse with mixed modes */
    reverse_with_both_modes(dst2, src, N);
    
    /* Verification */
    int ok4 = 1, ok5 = 1;
    for (i = 0; i < N; i++) {
        if (dst1[i] != 'X') ok4 = 0;
        if (dst2[i] != src[N-1-i]) ok5 = 0;
    }
    
    printf("Test 4 (fill with post-inc): %s\n", ok4 ? "PASS" : "FAIL");
    printf("Test 5 (reverse with mixed modes): %s\n", ok5 ? "PASS" : "FAIL");
}

/* Additional test with local pointer variables in loop */
static int __attribute__((noinline))
dot_product(const int *a, const int *b, int n) {
    const int *pa = a;
    const int *pb = b;
    int result = 0;
    
    /* Pattern: result += *pa++ * *pb++ */
    while (n-- > 0) {
        result += *pa++ * *pb++;  /* Two zero-offset memory reads */
    }
    return result;
}

static void test_dot_product(void) {
    const int N = 64;
    int a[N], b[N];
    int expected = 0;
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        expected += a[i] * b[i];
    }
    
    int result = dot_product(a, b, N);
    printf("Test 6 (dot product): %s (result=%d, expected=%d)\n",
           result == expected ? "PASS" : "FAIL", result, expected);
}

int main(void) {
    printf("Testing auto-inc-dec optimization patterns\n");
    printf("==========================================\n");
    
    test_int_patterns();
    test_char_patterns();
    test_dot_product();
    
    /* Use checksum to prevent optimization */
    printf("\nGlobal checksum: %d\n", g_checksum);
    
    return 0;
}
