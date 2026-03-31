/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 1024

/* Global volatile to prevent dead code elimination */
volatile int g_checksum = 0;

/* Noinline to ensure function boundaries are preserved */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ generates base+0 addressing */
    while (n--) {
        *dst++ = *src++;
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for reverse copy */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src generates base+0 addressing */
    while (n--) {
        *--dst = *--src;
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *src, int n) {
    int sum = 0;
    const int *p = src;
    
    /* Pattern: sum += *p++ generates base+0 addressing for read */
    while (n--) {
        sum += *p++;
    }
    return sum;
}

static void __attribute__((noinline))
fill_with_post_increment(int *dst, int value, int n) {
    /* Pattern: *dst++ = value generates base+0 addressing for write */
    while (n--) {
        *dst++ = value;
    }
}

static void __attribute__((noinline))
reverse_with_dual_pointers(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src + n - 1;
    
    /* Pattern: *d++ = *s-- uses two auto-decrement operations */
    while (n--) {
        *d++ = *s--;
    }
}

/* Test case 1: Simple forward copy with post-increment */
static int test_forward_copy(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    
    /* Initialize source with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Clear destination */
    memset(dst, 0, sizeof(dst));
    
    /* This should generate the target RTL pattern */
    copy_with_post_increment(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            return 0;
        }
    }
    return 1;
}

/* Test case 2: Reverse copy with pre-decrement */
static int test_reverse_copy(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 5 + 2;
    }
    
    memset(dst, 0, sizeof(dst));
    
    /* This should generate pre-decrement patterns */
    copy_with_pre_decrement(dst, src, ARRAY_SIZE);
    
    /* Verify reverse copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            return 0;
        }
    }
    return 1;
}

/* Test case 3: Sum calculation with post-increment */
static int test_sum_calculation(void) {
    int data[ARRAY_SIZE];
    int expected_sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i + 1;
        expected_sum += i + 1;
    }
    
    int calculated_sum = sum_with_post_increment(data, ARRAY_SIZE);
    g_checksum = calculated_sum; /* Store in volatile global */
    
    return (calculated_sum == expected_sum);
}

/* Test case 4: Array fill with post-increment */
static int test_array_fill(void) {
    int data[ARRAY_SIZE];
    const int FILL_VALUE = 0xABCD;
    
    memset(data, 0, sizeof(data));
    fill_with_post_increment(data, FILL_VALUE, ARRAY_SIZE);
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] != FILL_VALUE) {
            return 0;
        }
    }
    return 1;
}

/* Test case 5: String reversal with dual auto-modify */
static int test_string_reversal(void) {
    const char *original = "HelloWorld";
    int len = strlen(original);
    char reversed[64];
    
    reverse_with_dual_pointers(reversed, original, len);
    reversed[len] = '\0';
    
    /* Manually verify reversal */
    for (int i = 0; i < len; i++) {
        if (reversed[i] != original[len - 1 - i]) {
            return 0;
        }
    }
    return 1;
}

int main(void) {
    int passed = 0;
    int total = 0;
    
    printf("Testing auto-inc-dec optimization patterns...\n");
    
    /* Run all test cases */
    if (test_forward_copy()) { passed++; printf("  Forward copy: PASS\n"); }
    else { printf("  Forward copy: FAIL\n"); }
    total++;
    
    if (test_reverse_copy()) { passed++; printf("  Reverse copy: PASS\n"); }
    else { printf("  Reverse copy: FAIL\n"); }
    total++;
    
    if (test_sum_calculation()) { passed++; printf("  Sum calculation: PASS\n"); }
    else { printf("  Sum calculation: FAIL\n"); }
    total++;
    
    if (test_array_fill()) { passed++; printf("  Array fill: PASS\n"); }
    else { printf("  Array fill: FAIL\n"); }
    total++;
    
    if (test_string_reversal()) { passed++; printf("  String reversal: PASS\n"); }
    else { printf("  String reversal: FAIL\n"); }
    total++;
    
    printf("\nResults: %d/%d tests passed\n", passed, total);
    printf("Global checksum: %d\n", g_checksum);
    
    return (passed == total) ? 0 : 1;
}
