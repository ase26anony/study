/* test_auto_inc_dec.c
 * Program to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

/* Prevent inlining to ensure loops remain distinct for analysis */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ generates base+0 offset accesses */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
    dst += n;
    src += n;
    
    /* Pattern: *--dst = *--src generates base+0 offset accesses */
    while (n-- > 0) {
        *--dst = *--src;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static int __attribute__((noinline)) 
sum_with_post_increment(const int *arr, int n) {
    /* Pattern: sum += *ptr++ generates read with base+0 offset */
    int sum = 0;
    const int *ptr = arr;
    
    while (n-- > 0) {
        sum += *ptr++;  /* Memory read with zero offset */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    /* Pattern using both increment and decrement in same loop */
    int *start = arr;
    int *end = arr + n - 1;
    
    while (start < end) {
        int temp = *start;    /* Read with zero offset */
        *start++ = *end;      /* Write with zero offset, then increment */
        *end-- = temp;        /* Write with zero offset, then decrement */
    }
}

static void __attribute__((noinline))
fill_with_pre_increment(int *arr, int n, int value) {
    /* Pattern: *--ptr = value with pre-decrement */
    int *ptr = arr + n;
    
    while (n-- > 0) {
        *--ptr = value;  /* Pre-decrement then write with zero offset */
    }
}

/* Volatile to prevent dead code elimination */
static volatile int checksum = 0;

int main(void) {
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    int i;
    
    /* Initialize source array with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        src[i] = i * 3 + 1;
    }
    
    /* Clear destination */
    memset(dst, 0, sizeof(dst));
    
    printf("Testing auto-inc-dec optimization patterns...\n");
    
    /* Test 1: Post-increment copy */
    copy_with_post_increment(dst, src, ARRAY_SIZE);
    
    /* Verify copy */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Post-increment copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Post-increment copy\n");
    
    /* Test 2: Pre-decrement copy */
    memset(dst, 0, sizeof(dst));
    copy_with_pre_decrement(dst, src, ARRAY_SIZE);
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Pre-decrement copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Pre-decrement copy\n");
    
    /* Test 3: Sum with post-increment */
    int sum = sum_with_post_increment(src, ARRAY_SIZE);
    
    /* Calculate expected sum */
    int expected_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (sum != expected_sum) {
        printf("FAIL: Sum calculation mismatch: got %d, expected %d\n", 
               sum, expected_sum);
        return 1;
    }
    printf("PASS: Sum with post-increment: %d\n", sum);
    
    /* Test 4: Reverse with dual pointers */
    memcpy(dst, src, sizeof(src));
    reverse_with_dual_pointers(dst, ARRAY_SIZE);
    
    /* Verify reverse */
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != src[ARRAY_SIZE - 1 - i]) {
            printf("FAIL: Reverse mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Reverse with dual pointers\n");
    
    /* Test 5: Fill with pre-increment */
    fill_with_pre_increment(dst, ARRAY_SIZE, 42);
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (dst[i] != 42) {
            printf("FAIL: Fill mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Fill with pre-increment\n");
    
    /* Store checksum to volatile to prevent optimization */
    checksum = sum;
    
    printf("All tests passed! Auto-inc-dec patterns should be triggered.\n");
    return 0;
}
