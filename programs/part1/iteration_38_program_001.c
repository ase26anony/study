/* test_auto_inc_dec.c
 * Program to trigger GCC's auto-inc-dec pass for zero-offset memory access patterns
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 256
#define CHECKSUM_SEED 0x55AA

/* Prevent inlining to ensure loops remain distinct */
static void __attribute__((noinline)) 
copy_with_post_increment(int *dst, const int *src, int n) {
    /* Pattern: *dst++ = *src++ creates base+0 addressing */
    while (n-- > 0) {
        *dst++ = *src++;  /* Should generate (mem (reg)) with offset 0 */
    }
}

static void __attribute__((noinline)) 
copy_with_pre_decrement(int *dst, const int *src, int n) {
    /* Move pointers to end for pre-decrement pattern */
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
    
    /* Pattern: sum += *p++ creates base+0 read access */
    while (n-- > 0) {
        sum += *p++;  /* Memory read with zero offset */
    }
    return sum;
}

static void __attribute__((noinline))
reverse_with_dual_pointers(char *dst, const char *src, int n) {
    char *d = dst;
    const char *s = src + n - 1;
    
    /* Pattern: *d++ = *s-- uses both increment and decrement */
    while (n-- > 0) {
        *d++ = *s--;  /* Both accesses should be base+0 */
    }
}

static void __attribute__((noinline))
fill_with_post_increment(short *arr, short value, int n) {
    short *p = arr;
    
    /* Pattern: *p++ = value creates base+0 write access */
    while (n-- > 0) {
        *p++ = value;  /* Memory write with zero offset */
    }
}

/* Test different data types to explore various RTL patterns */
static void __attribute__((noinline))
mixed_operations(char *buf1, char *buf2, int n) {
    char *p1 = buf1;
    char *p2 = buf2;
    
    /* Multiple memory operations in same loop */
    while (n-- > 0) {
        /* Read from buf2, write to buf1 - both with zero offset */
        *p1++ = *p2++ + 1;
    }
}

/* Helper to verify results */
static int verify_copy(const int *a, const int *b, int n) {
    while (n-- > 0) {
        if (*a++ != *b++) return 0;
    }
    return 1;
}

int main(void) {
    int src_int[SIZE];
    int dst_int[SIZE];
    int dst_int2[SIZE];
    char src_char[SIZE];
    char dst_char[SIZE];
    short short_arr[SIZE];
    
    /* Initialize test data */
    for (int i = 0; i < SIZE; i++) {
        src_int[i] = i * 3 + 1;
        src_char[i] = 'A' + (i % 26);
    }
    
    printf("Testing auto-inc-dec patterns...\n");
    
    /* Test 1: Post-increment copy */
    memset(dst_int, 0, sizeof(dst_int));
    copy_with_post_increment(dst_int, src_int, SIZE);
    if (verify_copy(dst_int, src_int, SIZE)) {
        printf("✓ Post-increment copy passed\n");
    } else {
        printf("✗ Post-increment copy failed\n");
        return 1;
    }
    
    /* Test 2: Pre-decrement copy */
    memset(dst_int2, 0, sizeof(dst_int2));
    copy_with_pre_decrement(dst_int2, src_int, SIZE);
    if (verify_copy(dst_int2, src_int, SIZE)) {
        printf("✓ Pre-decrement copy passed\n");
    } else {
        printf("✗ Pre-decrement copy failed\n");
        return 1;
    }
    
    /* Test 3: Post-increment sum calculation */
    int sum = sum_with_post_increment(src_int, SIZE);
    int expected_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        expected_sum += src_int[i];
    }
    if (sum == expected_sum) {
        printf("✓ Post-increment sum passed: %d\n", sum);
    } else {
        printf("✗ Post-increment sum failed: got %d, expected %d\n", 
               sum, expected_sum);
        return 1;
    }
    
    /* Test 4: Reverse with dual pointers */
    memset(dst_char, 0, sizeof(dst_char));
    reverse_with_dual_pointers(dst_char, src_char, SIZE);
    
    int reverse_ok = 1;
    for (int i = 0; i < SIZE; i++) {
        if (dst_char[i] != src_char[SIZE - 1 - i]) {
            reverse_ok = 0;
            break;
        }
    }
    if (reverse_ok) {
        printf("✓ Reverse with dual pointers passed\n");
    } else {
        printf("✗ Reverse with dual pointers failed\n");
        return 1;
    }
    
    /* Test 5: Fill with post-increment */
    fill_with_post_increment(short_arr, 0x1234, SIZE);
    int fill_ok = 1;
    for (int i = 0; i < SIZE; i++) {
        if (short_arr[i] != 0x1234) {
            fill_ok = 0;
            break;
        }
    }
    if (fill_ok) {
        printf("✓ Fill with post-increment passed\n");
    } else {
        printf("✗ Fill with post-increment failed\n");
        return 1;
    }
    
    /* Test 6: Mixed operations */
    char buf1[SIZE], buf2[SIZE];
    for (int i = 0; i < SIZE; i++) {
        buf2[i] = i % 128;
    }
    mixed_operations(buf1, buf2, SIZE);
    
    int mixed_ok = 1;
    for (int i = 0; i < SIZE; i++) {
        if (buf1[i] != (char)((i % 128) + 1)) {
            mixed_ok = 0;
            break;
        }
    }
    if (mixed_ok) {
        printf("✓ Mixed operations passed\n");
    } else {
        printf("✗ Mixed operations failed\n");
        return 1;
    }
    
    printf("\nAll tests passed! The auto-inc-dec pass should have processed\n");
    printf("the zero-offset memory access patterns in the loops.\n");
    
    return 0;
}
