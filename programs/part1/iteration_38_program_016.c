/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 * Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c test_auto_inc_dec.c
 */

#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Use noinline to prevent inlining and preserve loop structure */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern 1: Post-increment with zero offset at memory access */
    while (n--) {
        *dst = *src;  /* Base + 0 offset pattern */
        dst++;
        src++;
    }
}

static void __attribute__((noinline)) 
copy_with_combined_post_inc(int *dst, const int *src, int n) {
    /* Pattern 2: Combined post-increment (more likely to generate desired RTL) */
    while (n--) {
        *dst++ = *src++;  /* Classic pointer increment pattern */
    }
}

static void __attribute__((noinline)) 
reverse_with_pre_decrement(int *dst, const int *src, int n) {
    /* Pattern 3: Pre-decrement with zero offset */
    dst += n;  /* Point to one past the end */
    src += n;
    
    while (n--) {
        *--dst = *--src;  /* Pre-decrement pattern */
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    /* Pattern 4: Read-only with post-increment */
    int sum = 0;
    const int *p = arr;
    
    while (n--) {
        sum += *p;  /* Zero offset read */
        p++;
    }
    return sum;
}

static void __attribute__((noinline))
fill_with_post_increment(int *arr, int value, int n) {
    /* Pattern 5: Write-only with post-increment */
    int *p = arr;
    
    while (n--) {
        *p = value;  /* Zero offset write */
        p++;
    }
}

/* Test different data types to explore various patterns */
static void __attribute__((noinline))
char_copy_with_inc(char *dst, const char *src, int n) {
    /* Pattern 6: char type with increment */
    while (n--) {
        *dst++ = *src++;
    }
}

/* Main test function that exercises all patterns */
int main(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    char csrc[ARRAY_SIZE];
    char cdst[ARRAY_SIZE];
    
    /* Initialize source arrays with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
        csrc[i] = (char)(i % 256);
    }
    
    /* Clear destination arrays */
    memset(dst, 0, sizeof(dst));
    memset(cdst, 0, sizeof(cdst));
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test 1: Simple post-increment copy */
    copy_with_post_increment(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Test 1 at index %d\n", i);
            return 1;
        }
    }
    printf("Test 1 passed: Simple post-increment\n");
    
    /* Reset and test combined pattern */
    memset(dst, 0, sizeof(dst));
    copy_with_combined_post_inc(dst, src, ARRAY_SIZE);
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Test 2 at index %d\n", i);
            return 1;
        }
    }
    printf("Test 2 passed: Combined post-increment\n");
    
    /* Test 3: Pre-decrement reverse copy */
    int src_reverse[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src_reverse[i] = i;
    }
    
    reverse_with_pre_decrement(dst, src_reverse, ARRAY_SIZE);
    
    /* Verify reverse copy */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src_reverse[ARRAY_SIZE - 1 - i]) {
            printf("FAIL: Test 3 at index %d\n", i);
            return 1;
        }
    }
    printf("Test 3 passed: Pre-decrement reverse\n");
    
    /* Test 4: Sum calculation */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    int expected_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (sum != expected_sum) {
        printf("FAIL: Test 4 sum mismatch: %d != %d\n", sum, expected_sum);
        return 1;
    }
    printf("Test 4 passed: Sum with post-increment\n");
    
    /* Test 5: Fill pattern */
    fill_with_post_increment(dst, 42, ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != 42) {
            printf("FAIL: Test 5 at index %d\n", i);
            return 1;
        }
    }
    printf("Test 5 passed: Fill with post-increment\n");
    
    /* Test 6: char type copy */
    char_copy_with_inc(cdst, csrc, ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (cdst[i] != csrc[i]) {
            printf("FAIL: Test 6 at index %d\n", i);
            return 1;
        }
    }
    printf("Test 6 passed: Char copy with increment\n");
    
    printf("\nAll tests passed! The auto-inc-dec pass should have processed these patterns.\n");
    printf("Check the dump file (test_auto_inc_dec.c.*.auto-inc-dec) for pass activity.\n");
    
    return 0;
}
