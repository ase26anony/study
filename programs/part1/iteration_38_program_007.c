/* test_auto_inc_dec.c
 * This program creates memory access patterns that should trigger
 * GCC's auto-increment/decrement optimization pass, specifically
 * the zero-offset pattern in find_inc().
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 1024
#define CHECKSUM_SEED 0x12345678

/* Prevent inlining to ensure loops remain intact for analysis */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates zero-offset accesses */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline))
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
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
        sum += *p++;  /* Should generate (mem (reg)) with offset 0 */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(int *arr, int n) {
    int *start = arr;
    int *end = arr + n - 1;
    
    /* Pattern: *start++ = *end-- creates zero-offset accesses */
    while (start < end) {
        int temp = *end;    /* Zero-offset read from end */
        *end-- = *start;    /* Zero-offset write to end, then decrement */
        *start++ = temp;    /* Zero-offset write to start, then increment */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(char *buf, char value, int n) {
    char *p = buf;
    
    /* Pattern: *p++ = value creates zero-offset write */
    while (n-- > 0) {
        *p++ = value;  /* Should generate (mem (reg)) with offset 0 */
    }
}

/* Main test driver */
int main(void) {
    int src[SIZE];
    int dst[SIZE];
    char buffer[SIZE];
    int i, checksum = CHECKSUM_SEED;
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        src[i] = i * 3 + 1;
        checksum ^= src[i];
    }
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test 1: Post-increment copy */
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
    
    /* Test 2: Pre-decrement copy */
    memset(dst, 0, sizeof(dst));
    copy_with_pre_decrement(dst, src, SIZE);
    
    /* Verify copy */
    for (i = 0; i < SIZE; i++) {
        if (dst[i] != src[i]) {
            printf("FAIL: Pre-decrement copy mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Pre-decrement copy\n");
    
    /* Test 3: Sum with post-increment */
    int calculated_sum = sum_with_post_increment(src, SIZE);
    int expected_sum = 0;
    for (i = 0; i < SIZE; i++) {
        expected_sum += src[i];
    }
    
    if (calculated_sum != expected_sum) {
        printf("FAIL: Sum calculation error: got %d, expected %d\n", 
               calculated_sum, expected_sum);
        return 1;
    }
    printf("PASS: Sum with post-increment\n");
    
    /* Test 4: Reverse with dual pointers */
    memcpy(dst, src, sizeof(src));
    reverse_with_dual_pointers(dst, SIZE);
    
    /* Verify reverse */
    for (i = 0; i < SIZE; i++) {
        if (dst[i] != src[SIZE - 1 - i]) {
            printf("FAIL: Reverse mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Reverse with dual pointers\n");
    
    /* Test 5: Char buffer fill */
    fill_with_post_increment(buffer, 'A', SIZE);
    
    /* Verify fill */
    for (i = 0; i < SIZE; i++) {
        if (buffer[i] != 'A') {
            printf("FAIL: Buffer fill mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("PASS: Char buffer fill\n");
    
    printf("\nAll tests passed! The auto-inc-dec pass should have seen\n");
    printf("multiple zero-offset memory access patterns in loops.\n");
    
    /* Use checksum to prevent dead code elimination */
    volatile int dummy = checksum;
    (void)dummy;
    
    return 0;
}
