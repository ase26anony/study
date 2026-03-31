/* test_auto_inc_dec.c
 * Program designed to trigger GCC's auto-inc-dec pass
 * Specifically targets the zero-offset memory access pattern
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRAY_SIZE 256

/* Global volatile to prevent optimization */
volatile int g_checksum = 0;

/* __attribute__((noinline)) prevents inlining so we can see the pattern */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates base+0 offset accesses */
    while (n--) {
        *dst++ = *src++;  /* Both accesses should have zero offset */
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src creates base+0 offset accesses */
    while (n--) {
        *--dst = *--src;  /* Both accesses should have zero offset */
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    int sum = 0;
    const int *p = arr;
    
    /* Pattern: sum += *p++ creates base+0 offset access */
    while (n--) {
        sum += *p++;  /* Memory access with zero offset */
    }
    
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: swap using *start++ and *end-- */
    while (start < end) {
        int temp = *start;
        *start++ = *end;    /* Zero offset access */
        *end-- = temp;      /* Zero offset access */
    }
}

/* More complex example with char pointers */
static void __attribute__((noinline))
string_copy_with_inc(char *dst, const char *src) {
    /* Pattern: *dst++ = *src++ until null terminator */
    while ((*dst++ = *src++)) {
        /* Both accesses have zero offset */
    }
}

/* Test function that combines multiple patterns */
static void __attribute__((noinline))
test_combined_patterns(void) {
    int src[ARRAY_SIZE];
    int dst1[ARRAY_SIZE];
    int dst2[ARRAY_SIZE];
    char str_src[ARRAY_SIZE];
    char str_dst[ARRAY_SIZE];
    
    /* Initialize source arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
        str_src[i] = 'A' + (i % 26);
    }
    str_src[ARRAY_SIZE - 1] = '\0';
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst1, src, ARRAY_SIZE);
    
    /* Test 2: Pre-decrement copy */
    copy_with_pre_decrement(dst2, src, ARRAY_SIZE);
    
    /* Test 3: Sum calculation with post-increment */
    int sum1 = sum_with_post_increment(dst1, ARRAY_SIZE);
    int sum2 = sum_with_post_increment(dst2, ARRAY_SIZE);
    
    /* Test 4: Reverse array using dual pointers */
    reverse_with_dual_pointers(dst1, ARRAY_SIZE);
    
    /* Test 5: String copy */
    string_copy_with_inc(str_dst, str_src);
    
    /* Verify results to prevent optimization */
    int verify_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        verify_sum += src[i];
    }
    
    /* Store in global volatile to ensure computation isn't eliminated */
    g_checksum = sum1 + sum2 + verify_sum + strlen(str_dst);
    
    /* Quick verification */
    int mismatch = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (dst1[ARRAY_SIZE - 1 - i] != src[i]) mismatch++;
        if (dst2[i] != src[i]) mismatch++;
    }
    
    if (mismatch == 0 && strcmp(str_src, str_dst) == 0) {
        printf("All patterns executed correctly. Checksum: %d\n", g_checksum);
    } else {
        printf("Verification failed. Mismatches: %d\n", mismatch);
    }
}

/* Main driver */
int main(void) {
    printf("Testing auto-increment/decrement patterns...\n");
    
    /* Call the test multiple times to increase chance of optimization */
    for (int i = 0; i < 3; i++) {
        test_combined_patterns();
    }
    
    return 0;
}
