/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-inc-dec pass
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Use noinline to prevent inlining and preserve loop structure */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n--) {
        *dst = *src;  /* Base + 0 offset pattern */
        dst++;        /* Post-increment */
        src++;        /* Post-increment */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates zero-offset accesses */
    while (n--) {
        dst--;        /* Pre-decrement */
        src--;        /* Pre-decrement */
        *dst = *src;  /* Base + 0 offset pattern */
    }
}

static int __attribute__((noinline))
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates zero-offset read */
    while (n--) {
        sum += *p;  /* Base + 0 offset pattern */
        p++;        /* Post-increment */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: *start++ = *end-- creates zero-offset accesses */
    while (start < end) {
        int temp = *end;  /* Base + 0 offset pattern */
        *end = *start;    /* Base + 0 offset pattern */
        *start = temp;    /* Base + 0 offset pattern */
        start++;
        end--;
    }
}

/* Another variation with char pointers (often generates simpler RTL) */
static void __attribute__((noinline))
copy_chars_with_inc(char *dst, const char *src, int n) {
    /* Explicit pointer arithmetic in loop */
    char *d = dst;
    const char *s = src;
    
    while (n--) {
        *d = *s;  /* Base + 0 offset pattern */
        d = d + 1;
        s = s + 1;
    }
}

/* Test function that combines multiple patterns */
static int __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    int result = 0;
    
    /* Initialize source array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Clear destination arrays */
    memset(dst1, 0, sizeof(dst1));
    memset(dst2, 0, sizeof(dst2));
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst1, src, ARRAY_SIZE);
    
    /* Test 2: Pre-decrement copy (reverse order) */
    copy_with_pre_decrement(dst2, src, ARRAY_SIZE);
    
    /* Test 3: Sum calculation with post-increment */
    int sum1 = sum_with_post_increment(src, ARRAY_SIZE);
    int sum2 = sum_with_post_increment(dst1, ARRAY_SIZE);
    
    /* Test 4: Array reversal */
    int temp[ARRAY_SIZE];
    memcpy(temp, src, sizeof(temp));
    reverse_with_dual_pointers(temp, ARRAY_SIZE);
    
    /* Verify results */
    int valid = 1;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[i] != src[i]) valid = 0;
        if (dst2[ARRAY_SIZE - 1 - i] != src[i]) valid = 0;
        if (temp[ARRAY_SIZE - 1 - i] != src[i]) valid = 0;
    }
    
    if (sum1 != sum2) valid = 0;
    
    /* Additional char array test */
    char cs[256], cd[256];
    for (int i = 0; i < 256; i++) cs[i] = i;
    copy_chars_with_inc(cd, cs, 256);
    if (memcmp(cs, cd, 256) != 0) valid = 0;
    
    return valid;
}

int main(void) {
    int success = test_combined_patterns();
    
    if (success) {
        printf("All tests passed. The auto-inc-dec pass should have processed the patterns.\n");
        printf("Compile with: gcc -O2 -fdump-rtl-auto-inc-dec -c test_auto_inc_dec.c\n");
        printf("Check the generated .auto-inc-dec file for pass activity.\n");
    } else {
        printf("Test failed - logic error in test code.\n");
    }
    
    return success ? 0 : 1;
}
